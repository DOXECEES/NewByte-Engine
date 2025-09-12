// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "Subsystems.hpp"

void Subsystems::Init(const HWND& hwnd)
{    
    //renderer2d = new Renderer2D();
    renderer = createRef<nb::Renderer::Renderer>(hwnd, nb::Core::EngineSettings::getGraphicsAPI());
    keyboard = createRef<nb::Input::Keyboard>();
    mouse = createRef<nb::Input::Mouse>();
}

Subsystems::~Subsystems()
{
    //renderer.reset();
}