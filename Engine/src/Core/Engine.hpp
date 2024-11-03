#ifndef SRC_CORE_ENGINE_HPP
#define SRC_CORE_ENGINE_HPP

#include "../Subsystems.hpp"

#include <memory>

namespace nb
{
    namespace Core
    {
        class Engine
        {
        public:
            Engine(const HWND& hwnd);
            ~Engine() = default;
            
            void run();


        private:
            std::unique_ptr<Subsystems> subSystems = std::make_unique<Subsystems>();
            Ref<nb::Renderer::Renderer> renderer = nullptr;
            bool isRunning = true;
        };
    };
};

#endif