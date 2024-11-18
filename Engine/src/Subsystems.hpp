#pragma once

#include "Renderer/Renderer2D.hpp"
#include "Renderer/Renderer.hpp"
#include "Core/EngineSettings.hpp"
#include "Window.hpp"

#include "Input/Keyboard.hpp"
#include "Input/Mouse.hpp"

#include "Core.hpp"

class Subsystems
{
public:
    Subsystems() = default;

    void Init(const HWND& hwnd);

//    inline Renderer2D *GetRenderer2D() const noexcept
//    {
//        return renderer2d;
//    }

    inline Ref<nb::Renderer::Renderer> getRenderer() const noexcept
    {
        return renderer;
    }

    inline Ref<nb::Input::Keyboard> getKeyboard() const noexcept
    {
        return keyboard;
    }

    inline Ref<nb::Input::Mouse> getMouse() const noexcept
    {
        return mouse;
    }

    ~Subsystems();

private:
    // subsystems list
    //Renderer2D *renderer2d               = nullptr;
    Ref<nb::Renderer::Renderer> renderer = nullptr;
    Ref<nb::Input::Keyboard> keyboard    = nullptr;
    Ref<nb::Input::Mouse> mouse          = nullptr;
    //
};
