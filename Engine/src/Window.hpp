#ifndef SRC_WINDOW_HPP
#define SRC_WINDOW_HPP

#include <Windows.h>

#include <functional>

#include "Renderer/Renderer.hpp"

class Window
{

public:
    Window(HINSTANCE inst, WNDPROC handler);

    bool render();

    void exit(LPCWSTR text)
    {
        MessageBox(hwnd, text, NULL, MB_OK);
        PostQuitMessage(0);
    }

    inline HWND getHandle() const noexcept
    {
        return hwnd;
    }

private:
    HWND hwnd = {};
    WNDCLASS wc = {};
    nb::Renderer::Renderer* renderer;
};

#endif