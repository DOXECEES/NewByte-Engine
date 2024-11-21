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

        void resize(const int newWidth, const int newHeigth) noexcept;

        static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    
    private:
        std::shared_ptr<nb::Core::Engine> engine;
        HWND hwnd;
    };
};

#endif


