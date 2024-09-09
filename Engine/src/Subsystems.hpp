#pragma once

#include "Renderer/Renderer2D.hpp"
#include "Window.hpp"

class Subsystems
{
public:
    Subsystems() = default;
    void Init();

    inline Renderer2D *GetRenderer2D() const noexcept
    {
        return renderer2d;
    }

    ~Subsystems();

private:
    // sybsustems list
    Renderer2D *renderer2d = nullptr;

    //
};
