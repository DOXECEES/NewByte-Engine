// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "Window.hpp"

#include "Core/EngineSettings.hpp"

#include "Core.hpp"

Window::Window(HINSTANCE inst, WNDPROC handler)
{
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = handler;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = inst;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(6);
    wc.lpszMenuName = 0;
    wc.lpszClassName = L"class";

    RegisterClass(&wc);
    hwnd = CreateWindow(L"class", L"Direct2D init",
                        WS_OVERLAPPEDWINDOW, 100, 100, nb::Core::EngineSettings::getWidth(),
                        nb::Core::EngineSettings::getHeight(), 
                        NULL, NULL,
                        inst, NULL);
						
	//renderer = new nb::Renderer::Renderer(hwnd, nb::Core::EngineSettings::getGraphicsAPI());
    ShowCursor(NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
}

// no longer used
bool Window::render()
{
    MSG msg;

    if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
            return false;
      
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

	return true;
}