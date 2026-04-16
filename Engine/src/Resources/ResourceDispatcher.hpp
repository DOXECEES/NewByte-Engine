#ifndef SRC_RESOURCES_RESOURCEDISPATCHER_HPP
#define SRC_RESOURCES_RESOURCEDISPATCHER_HPP

#include <memory>
#include <string>
#include <string_view>
#include <functional>

namespace nb
{
    namespace Loaders { class Node; }
    namespace Serialize { class IArchive; }
    namespace Reflect { struct TypeInfo; }
}

namespace nb::Resources
{

    class ResourceDispatcher
    {
    public:
        using ComplexLoader = std::function<void(void* fieldPtr, const nb::Loaders::Node& node)>;
        using ComplexSerializer = std::function<bool(nb::Serialize::IArchive* archive, const char* name, void* fieldPtr)>;

        static constexpr std::string_view TYPE_MESH_PTR = "std::shared_ptr<nb::Renderer::Mesh>";

        ResourceDispatcher();
        ~ResourceDispatcher(); 

        ResourceDispatcher(const ResourceDispatcher&) = delete;
        ResourceDispatcher& operator=(const ResourceDispatcher&) = delete;

        bool serialize(
            nb::Serialize::IArchive* archive,
            const char*              fieldName,
            void*                    fieldPtr,
            nb::Reflect::TypeInfo*   typeInfo
        );

        void dispatch(
            void*                    fieldPtr,
            nb::Reflect::TypeInfo*   typeInfo,
            const nb::Loaders::Node& node
        );

    private:
        struct Impl;
        std::unique_ptr<Impl> impl;
    };
}

#endif // SRC_RESOURCES_RESOURCEDISPATCHER_HPP