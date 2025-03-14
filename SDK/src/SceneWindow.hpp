#ifndef SRC_SCENEWINDOW_HPP
#define SRC_SCENEWINDOW_HPP

#include <Windows.h>

#include <../../Engine/src/Core/Engine.hpp>

namespace Editor
{
    class SceneWindow
    {
        static constexpr auto WIDTH_PROPORTION = 0.7f;
        static constexpr auto HEIGHT_PROPORTION = 0.7f;
        static constexpr auto CLASS_NAME = L"MDI_SCENE_CLASS";

    public:
    
        SceneWindow(const HWND& parentHwnd, std::shared_ptr<nb::Core::Engine> engine);
        ~SceneWindow() = default;

        inline const HWND &getHandle() const noexcept { return hwnd; };
        nb::Input::MouseDelta peekMouseDelta() noexcept;
        inline nb::Input::MouseButtons getMouseButtons() const noexcept { return mouseButtons; };

        void resize(const int newWidth, const int newHeigth) noexcept;

        static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
        void setEngine(std::shared_ptr<nb::Core::Engine> e)
        {
            engine = e;
            Sengine = e;
        }
    
    private:
        std::shared_ptr<nb::Core::Engine> engine;
        HWND hwnd;

        inline static nb::Input::MouseDelta mouseDelta = {};
        inline static nb::Input::MouseButtons mouseButtons = {};

        static std::shared_ptr<nb::Core::Engine> Sengine;


        static bool isPrevClick;
    };
};

#endif


