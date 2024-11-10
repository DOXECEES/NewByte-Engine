#ifndef SRC_CORE_ENGINE_HPP
#define SRC_CORE_ENGINE_HPP

#include "../Subsystems.hpp"

#include "../Input/Input.hpp"
#include "../Input/Keyboard.hpp"
#include "../Input/Mouse.hpp"

#include "../Renderer/Camera.hpp"

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

            inline static const HWND& getLinkedHwnd() noexcept { return hwnd; };

        private:
            std::unique_ptr<Subsystems> subSystems = std::make_unique<Subsystems>();
            Ref<nb::Renderer::Renderer> renderer = nullptr;
            Ref<nb::Input::Keyboard> keyboard    = nullptr;
            Ref<nb::Input::Mouse> mouse          = nullptr;
            bool isRunning = true;

            // temp
            Ref<nb::Input::Input> input          = nullptr;

            static HWND hwnd;
        };
    };
};

#endif