#pragma once

#include <Windows.h>

#include <functional>

class Window
{

public:
    Window(HINSTANCE inst, WNDPROC handler);

    bool render();

    HWND hwnd = {};

private:
    WNDCLASS wc = {};
};
