// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "ShaderFactory.hpp"
#include "Debug.hpp"

namespace nb
{
    namespace Loaders
    {
        namespace Factory
        {
            Ref<nb::Resource::IResource> ShaderFactory::create(
                const std::filesystem::path& path,
                nbstl::Span<std::string>     params
            ) const
            {
                switch (Core::EngineSettings::getGraphicsAPI())
                {
                case Core::GraphicsAPI::OPENGL:
                {
                    auto rm = nb::ResMan::ResourceManager::getInstance();

                    auto shaders = rm->getResource<Json>("Assets/shaders/config.nbsd");
                    if (!shaders)
                    {
                        Error::ErrorManager::instance().report(Error::Type::FATAL, "Shader config not found!");
                        return nullptr;
                    }

                    auto str = path.string();

                    //if (!shaders->contains(str))
                    //{
                    //    Error::ErrorManager::instance().report(
                    //        Error::Type::FATAL, "Shader '{}' not found in config.nbsd", str
                    //    );
                    //    return nullptr;
                    //}

                    std::vector<std::filesystem::path> s = (*shaders)[str]["sources"].getArray<std::filesystem::path>();

                    return createRef<nb::OpenGl::OpenGlShader>(s, str, params);
                }
                case Core::GraphicsAPI::DIRECTX:
                    assert("Not availiable now");
                    return nullptr;
                case Core::GraphicsAPI::VULKAN:
                    assert("Not availiable now");
                    return nullptr;
                default:
                    assert("Invalid API type");
                    return {};
                }
            }
        };
    };
};
