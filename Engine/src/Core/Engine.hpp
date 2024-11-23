#ifndef SRC_CORE_ENGINE_HPP
#define SRC_CORE_ENGINE_HPP

#ifdef _WIN32
#   ifdef ENGINE_EXPORTS
#       define ENGINE_API __declspec(dllexport)
#   else
#       define ENGINE_API __declspec(dllimport)
#   endif
#else
#   define ENGINE_API
#endif



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
            
            bool run(const MSG& msg);
            void setHandleInput(bool var) { handleInput = var; };

            inline static const HWND& getLinkedHwnd() noexcept { return hwnd; };

        private:
            bool isEditorMode = true;

            std::unique_ptr<Subsystems> subSystems = std::make_unique<Subsystems>();
            Ref<nb::Renderer::Renderer> renderer = nullptr;
            Ref<nb::Input::Keyboard> keyboard    = nullptr;
            Ref<nb::Input::Mouse> mouse          = nullptr;
            bool isRunning = true;
            bool handleInput = true;
            
            bool isSampling = true;

            // temp
            Ref<nb::Input::Input> input          = nullptr;

            static HWND hwnd;
        };
    };
};

#endif