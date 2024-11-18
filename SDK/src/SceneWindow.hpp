#ifndef SRC_SCENEWINDOW_HPP
#define SRC_SCENEWINDOW_HPP

#include <Windows.h>

namespace Editor
{
    class SceneWindow
    {
        static constexpr auto CLASS_NAME = L"SCENE_WINDOW";

    public:
    
        SceneWindow(const HWND& parentHwnd);
        ~SceneWindow() = default;

        inline const HWND &getHandle() const noexcept { return hwnd; };

        static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    
    private:
    
        HWND hwnd;
    };
};

#endif


