#include "ResourceDispatcher.hpp"

#include "Error/ErrorManager.hpp"
#include "Loaders/JSON/Node.hpp"
#include "Renderer/Objects/Objects.hpp"
#include "Serialize/IArchive.hpp"

#include <Reflection/Reflection.hpp>
#include <unordered_map>

namespace nb::Resources
{
    static constexpr std::string_view KEY_TYPE       = "type";
    static constexpr std::string_view KEY_PARAMETERS = "parameters";
    static constexpr std::string_view KEY_RADIUS     = "radius";
    static constexpr std::string_view KEY_X_SEGMENTS   = "xSegments";
    static constexpr std::string_view KEY_Y_SEGMENTS   = "ySegments";



    static constexpr std::string_view KEY_MAJOR_RADIUS = "minorRadius";
    static constexpr std::string_view KEY_MINOR_RADIUS = "majorRadius";


    static constexpr std::string_view VALUE_SPHERE   = "sphere";
    static constexpr std::string_view VALUE_CUBE     = "cube";

    static constexpr std::string_view VALUE_TORUS    = "torus";


    struct ResourceDispatcher::Impl
    {
        std::unordered_map<std::string, ComplexSerializer> serializers;
        std::unordered_map<std::string, ComplexLoader>     loaders;

        void registerLoader(const std::string& typeName, ComplexLoader loader)
        {
            if (!loader)
            {
                nb::Error::ErrorManager::instance()
                    .report(nb::Error::Type::WARNING, "Registering null loader")
                    .with("type", typeName);
                return;
            }
            loaders[typeName] = std::move(loader);
        }

        void registerSerializer(const std::string& typeName, ComplexSerializer serializer)
        {
            serializers[typeName] = std::move(serializer);
        }

        void loadComplexMesh(void* fieldPtr, const nb::Loaders::Node& node)
        {
            if (!fieldPtr)
            {
                nb::Error::ErrorManager::instance()
                    .report(nb::Error::Type::FATAL, "Field pointer is null in loadComplexMesh");
                return;
            }

            const std::string meshType = node[KEY_TYPE.data()].get<std::string>();
            auto* targetField = static_cast<std::shared_ptr<nb::Renderer::Mesh>*>(fieldPtr);

            if (meshType == VALUE_SPHERE)
            {
                const auto& params   = node[KEY_PARAMETERS.data()];
                const float radius   = params[KEY_RADIUS.data()].get<float>();
                const int   xSegments = static_cast<int>(params[KEY_X_SEGMENTS.data()].get<float>());
                const int   ySegments  = static_cast<int>(params[KEY_Y_SEGMENTS.data()].get<float>());

                nb::Error::ErrorManager::instance()
                    .report(nb::Error::Type::INFO, "Generating primitive sphere")
                    .with("r", radius)
                    .with("xSeg", xSegments)
                    .with("ySeg", ySegments);

                *targetField = nb::Renderer::PrimitiveGenerators::createSphere(radius, xSegments, ySegments);
            }
            else if (meshType == VALUE_CUBE)
            {
                nb::Error::ErrorManager::instance()
                    .report(nb::Error::Type::INFO, "Generating primitive cube");

                *targetField = nb::Renderer::PrimitiveGenerators::createCube();
            }
            else if (meshType == VALUE_TORUS)
            {
                const auto& params   = node[KEY_PARAMETERS.data()];
                const float major   = params[KEY_MAJOR_RADIUS.data()].get<float>();
                const float minor    = params[KEY_MINOR_RADIUS.data()].get<float>();

                const float xSegments = (params[KEY_X_SEGMENTS.data()].get<float>());
                const float ySegments = (params[KEY_Y_SEGMENTS.data()].get<float>());

                nb::Error::ErrorManager::instance()
                    .report(nb::Error::Type::INFO, "Generating primitive torus")
                    .with("minor", minor)
                    .with("major", major)
                    .with("xSeg", xSegments)
                    .with("ySeg", ySegments);

                *targetField =
                    nb::Renderer::PrimitiveGenerators::createTorus({xSegments, ySegments}, major, minor);
            }
            else
            {
                nb::Error::ErrorManager::instance()
                    .report(nb::Error::Type::WARNING, "Unknown mesh type")
                    .with("type", meshType);
            }
        }

        bool serializeComplexMesh(nb::Serialize::IArchive* archive, const char* name, void* fieldPtr)
        {
            auto* meshPtr = static_cast<std::shared_ptr<nb::Renderer::Mesh>*>(fieldPtr);
            if (!meshPtr || !(*meshPtr)) return false;

            const auto& mesh = *meshPtr;
            if (mesh->isPrimitive())
            {
                archive->beginObject(name);
                const auto& descOpt = mesh->getPrimitiveDescriptor();
                if (!descOpt.has_value())
                {
                    nb::Error::ErrorManager::instance()
                        .report(nb::Error::Type::WARNING, "Mesh primitive lacks descriptor");
                    archive->endObject();
                    return false;
                }

                const auto& desc = descOpt.value();
                std::string typeValue = desc.type;
                archive->value(KEY_TYPE.data(), typeValue);

                archive->beginObject(KEY_PARAMETERS.data());
                for (auto [paramName, paramValue] : desc.parameters)
                {
                    float val = paramValue;
                    archive->value(paramName.c_str(), val);
                }
                archive->endObject(); 
                archive->endObject();
                return true;
            }
            return false;
        }
    };

    ResourceDispatcher::ResourceDispatcher() 
        : impl(std::make_unique<Impl>())
    {
        impl->registerLoader(
            std::string(TYPE_MESH_PTR),
            [this](void* fieldPtr, const nb::Loaders::Node& node) {
                impl->loadComplexMesh(fieldPtr, node);
            }
        );

        impl->registerSerializer(
            std::string(TYPE_MESH_PTR),
            [this](nb::Serialize::IArchive* archive, const char* name, void* fieldPtr) {
                return impl->serializeComplexMesh(archive, name, fieldPtr);
            }
        );
    }

    ResourceDispatcher::~ResourceDispatcher() = default;

    bool ResourceDispatcher::serialize(
        nb::Serialize::IArchive* archive,
        const char*              fieldName,
        void*                    fieldPtr,
        nb::Reflect::TypeInfo*   typeInfo
    )
    {
        if (!archive || !fieldPtr || !typeInfo) return false;

        auto it = impl->serializers.find(typeInfo->name);
        if (it != impl->serializers.end())
        {
            return it->second(archive, fieldName, fieldPtr);
        }
        return false;
    }

    void ResourceDispatcher::dispatch(
        void*                    fieldPtr,
        nb::Reflect::TypeInfo*   typeInfo,
        const nb::Loaders::Node& node
    )
    {
        if (!typeInfo)
        {
            nb::Error::ErrorManager::instance()
                .report(nb::Error::Type::FATAL, "Null type info in dispatch");
            return;
        }

        auto it = impl->loaders.find(typeInfo->name);
        if (it != impl->loaders.end())
        {
            it->second(fieldPtr, node);
        }
        else
        {
            nb::Error::ErrorManager::instance()
                .report(nb::Error::Type::WARNING, "No complex loader for type")
                .with("typeName", typeInfo->name);
        }
    }
}