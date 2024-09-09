#include "Subsystems.hpp"

void Subsystems::Init()
{
    renderer2d = new Renderer2D();
}

Subsystems::~Subsystems()
{
    delete renderer2d;
}