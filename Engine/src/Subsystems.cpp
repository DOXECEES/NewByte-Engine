#include "Subsystems.hpp"

void Subsystems::Init()
{
    nb::Core::EngineSettings::deserialize();
    
    renderer2d = new Renderer2D();
}

Subsystems::~Subsystems()
{
    delete renderer2d;
}