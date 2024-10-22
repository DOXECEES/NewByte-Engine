#include "Renderer.hpp"

nb::Renderer::Renderer::Renderer(HWND hwnd,GraphicsAPI apiType) noexcept
{
    switch (apiType)
    {
    case GraphicsAPI::OPENGL:
        api = nb::OpenGl::OpenGLRender::create(hwnd);
        if(!api)
            std::abort();
        break;
    case GraphicsAPI::DIRECTX:
        NB_FALLTHROUGH;
    case GraphicsAPI::VULKAN:
        break;
    }
}