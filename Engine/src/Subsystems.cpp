#include "Subsystems.hpp"

void Subsystems::Init(const HWND& hwnd)
{    
    renderer2d = new Renderer2D();
    renderer = createRef<nb::Renderer::Renderer>(hwnd, nb::Core::EngineSettings::getGraphicsAPI());
    keyboard = createRef<nb::Input::Keyboard>();
    mouse = createRef<nb::Input::Mouse>();
}

Subsystems::~Subsystems()
{
    delete renderer2d;
}