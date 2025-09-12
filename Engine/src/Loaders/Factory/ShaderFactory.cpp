// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "ShaderFactory.hpp"

namespace nb
{
    namespace Loaders
    {
        namespace Factory
        {
            Ref<nb::Resource::IResource> ShaderFactory::create(const std::filesystem::path &path) const
            {
                switch (Core::EngineSettings::getGraphicsAPI())
                {
                case Core::GraphicsAPI::OPENGL:
                {
                    auto rm = nb::ResMan::ResourceManager::getInstance();
                    auto shaders = rm->getResource<Json>("C:\\rep\\Hex\\NewByte-Engine\\build\\Engine\\Debug\\shaders\\config.nbsd");
                    auto str = path.string();
                    std::vector<std::filesystem::path> s = (*shaders)[str]["sources"].getArray<std::filesystem::path>();
                    return createRef<nb::OpenGl::OpenGlShader>(s);
                }
                case Core::GraphicsAPI::DIRECTX:
                    assert("Not availiable now");
                    return createRef<nb::OpenGl::OpenGlShader>(path);
                case Core::GraphicsAPI::VULKAN:
                    assert("Not availiable now");
                    return createRef<nb::OpenGl::OpenGlShader>(path);
                default:
                    assert("Invalid API type");
                    return {};
                }
            }
        };
    };
};
