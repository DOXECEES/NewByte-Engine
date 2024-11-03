#include "Subsystems.hpp"

void Subsystems::Init(const HWND& hwnd)
{
    nb::Core::EngineSettings::deserialize();
    
    renderer2d = new Renderer2D();
    renderer = createRef<nb::Renderer::Renderer>(hwnd, nb::Core::EngineSettings::getGraphicsAPI());
}

Subsystems::~Subsystems()
{
    delete renderer2d;
}