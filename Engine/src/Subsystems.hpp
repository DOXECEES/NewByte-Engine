#pragma once

#include "Core/EngineSettings.hpp"
#include "Physics/Physics.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/Renderer2D.hpp"
#include "Window.hpp"

#include "Input/Keyboard.hpp"
#include "Input/Mouse.hpp"

#include "Core.hpp"

class Subsystems
{
public:
    Subsystems() = default;

    void Init(HWND hwnd);

   

    const nb::Physics::PhysicsSystem& getPhysicsSystem() const noexcept;

    nb::Physics::PhysicsSystem& getPhysicsSystem() noexcept;

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

    ~Subsystems() noexcept = default;

private:
    // subsystems list
    // Renderer2D *renderer2d               = nullptr;
    Ref<nb::Renderer::Renderer>     renderer = nullptr;
    Ref<nb::Input::Keyboard>        keyboard = nullptr;
    Ref<nb::Input::Mouse>           mouse    = nullptr;
    nb::Physics::PhysicsSystem      physics;
    //
};
