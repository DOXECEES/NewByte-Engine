#include "SceneWindow.hpp"
#include "UiStore.hpp"

namespace Editor
{

    SceneWindow::SceneWindow(const HWND &parentHwnd)
    {
        WNDCLASSEX wcex = {};
        HINSTANCE hInst = GetModuleHandle(NULL);

        if (!GetClassInfoEx(hInst, CLASS_NAME, &wcex))
        {
            wcex.cbSize = sizeof(WNDCLASSEX);
            wcex.style          = CS_HREDRAW | CS_VREDRAW;
            wcex.lpfnWndProc    = WndProc;
            wcex.cbClsExtra     = 0;
            wcex.cbWndExtra     = 0;
            wcex.hInstance      = hInst;
            wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
            wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
            wcex.lpszMenuName   = NULL;
            wcex.lpszClassName  = CLASS_NAME;
            wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));


            if (!RegisterClassEx(&wcex))
            {
                auto i = GetLastError();
                MessageBox(NULL, L"Failed to register window class", L"Error", MB_ICONERROR);
            }
        }

        hwnd = CreateWindowEx(NULL,
                              CLASS_NAME,
                              NULL,
                              WS_OVERLAPPEDWINDOW | WS_CHILD,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              parentHwnd,
                              NULL,
                              hInst,
                              NULL);

        //RECT parentRect;
        //GetClientRect(parentHwnd, &parentRect);
        //SetWindowPos(hwnd, HWND_TOP, parentRect.right / 2 -100, 0, 200, 100, 0);

        
    }

    LRESULT SceneWindow::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        HDC hdc;
        PAINTSTRUCT ps;
        RECT rect;

        switch (message)
        {
            case WM_SIZE:
            {
                Editor::UiStore::move(1, LOWORD(lParam), HIWORD(lParam));
                // GetClientRect(childhwnd, &rect);

                // SetWindowPos(childhwnd, HWND_TOP, 100, 200, LOWORD(lParam), HIWORD(lParam), 0);
                // return 0;
            }
        }
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
};