#include "Renderer.hpp"

nb::Renderer::Renderer::Renderer(HWND hwnd, nb::Core::GraphicsAPI apiType) noexcept
{
    switch (apiType)
    {
    case nb::Core::GraphicsAPI::OPENGL:
        api = nb::OpenGl::OpenGLRender::create(hwnd);
        if(!api)
            std::abort();
        break;
    case nb::Core::GraphicsAPI::DIRECTX:
        NB_FALLTHROUGH;
    case nb::Core::GraphicsAPI::VULKAN:
        break;
    }
}