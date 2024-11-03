#include "Engine.hpp"


namespace nb
{
    namespace Core
    {
        Engine::Engine(const HWND& hwnd)
        {
            subSystems->Init(hwnd);
            renderer = subSystems->getRenderer();
        }

        void Engine::run()
        {
            renderer->render();
        }
    };
};
