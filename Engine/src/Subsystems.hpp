#pragma once

#include "Renderer/Renderer2D.hpp"
#include "Renderer/Renderer.hpp"
#include "Core/EngineSettings.hpp"
#include "Window.hpp"

#include "Core.hpp"

class Subsystems
{
public:
    Subsystems() = default;
    void Init(const HWND& hwnd);

    inline Renderer2D *GetRenderer2D() const noexcept
    {
        return renderer2d;
    }

    inline Ref<nb::Renderer::Renderer> getRenderer() const noexcept
    {
        return renderer;
    }

    

    ~Subsystems();

private:
    // subsystems list
    Renderer2D *renderer2d = nullptr;
    Ref<nb::Renderer::Renderer> renderer = nullptr;

    //
};
