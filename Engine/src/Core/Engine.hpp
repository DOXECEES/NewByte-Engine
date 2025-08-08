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

#include "../Math/RayCast/RayPicker.hpp"
#include "../Utils/Timer.hpp"

#include <memory>

namespace nb
{
    namespace Core
    {
        class Engine
        {
        public:

            enum class Mode
            {
                EDITOR,
                GAME
            };

            Engine(const HWND& hwnd);
            ~Engine() = default;

            void bufferizeInput(const MSG& msg) noexcept;
            void processInput() noexcept;
            void setHandleInput(bool var) { handleInput = var; };

            bool run(Input::MouseDelta mouseDelta, Input::MouseButtons buttons);
            void handleGameMode(nb::Math::Vector3<float> &camDir, float deltaTime) noexcept;
            void handleEditorMode() noexcept;


            void rayPick(const uint32_t x, const uint32_t y) noexcept;
            
            Mode getMode() const noexcept;

            inline static const HWND& getLinkedHwnd() noexcept { return hwnd; };
            inline Math::Vector3<float> getCameraPos() { return renderer->getCamera()->getPosition(); };
            inline Math::Vector3<float> getCameraDirection() { return renderer->getCamera()->getDirection(); };
            inline std::shared_ptr<Renderer::SceneGraph> getScene() const noexcept { return renderer->getScene(); };
            inline Ref<nb::Renderer::Renderer> getRenderer() noexcept { return renderer; };


        private:
            Mode                        mode            = Mode::EDITOR;

            std::unique_ptr<Subsystems> subSystems      = std::make_unique<Subsystems>();
            Ref<nb::Renderer::Renderer> renderer        = nullptr;
            Ref<nb::Input::Keyboard>    keyboard        = nullptr;
            Ref<nb::Input::Mouse>       mouse           = nullptr;
            bool                        isRunning       = true;
            bool                        handleInput     = true;
            
            bool                        isSampling      = true;
            nb::Renderer::Camera        cam;
            // temp
            Ref<nb::Input::Input>       input           = nullptr;

            Input::MouseDelta           mouseDelta      = {};
            Input::MouseButtons         buttons         = {};

            inline static HWND          hwnd            = nullptr;
        };
    };
};

#endif