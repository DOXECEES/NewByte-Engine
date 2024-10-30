#include "ShaderFactory.hpp"
#include "../../Renderer/OpenGL/OpenGlShader.hpp"

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
                    return createRef<nb::OpenGl::OpenGlShader>(path);
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
