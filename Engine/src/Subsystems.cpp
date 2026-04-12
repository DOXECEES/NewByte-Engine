// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "Subsystems.hpp"

void Subsystems::Init(HWND hwnd)
{    
    //renderer2d = new Renderer2D();
    renderer = createRef<nb::Renderer::Renderer>(hwnd, nb::Core::EngineSettings::getGraphicsAPI());
    keyboard = createRef<nb::Input::Keyboard>();
    mouse = createRef<nb::Input::Mouse>();
    physics.init();
}

const nb::Physics::PhysicsSystem& Subsystems::getPhysicsSystem() const noexcept
{
    return physics;
}

nb::Physics::PhysicsSystem& Subsystems::getPhysicsSystem() noexcept
{
    return physics;
}

