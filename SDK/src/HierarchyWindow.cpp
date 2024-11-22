#include "HierarchyWindow.hpp"

namespace Editor
{
    HierarchyWindow::HierarchyWindow(const HWND &parentHwnd)
    {
        WNDCLASS wc = {};
        wc.lpfnWndProc = WndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = CLASS_NAME;
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

        if (!RegisterClass(&wc))
        {
            MessageBox(nullptr, L"Failed to register MDI child class!", L"Error", MB_ICONERROR);
        }

        MDICREATESTRUCT mcs = {};

        mcs.x = 0;
        mcs.y = 0;
        mcs.cx = 0;
        mcs.cy = 0;
        mcs.szTitle = L"HIERARCHY";
        mcs.szClass = CLASS_NAME;
        mcs.hOwner = GetModuleHandle(nullptr);
        mcs.style = WS_OVERLAPPEDWINDOW;

        hwnd = (HWND)SendMessage(parentHwnd, WM_MDICREATE, 0, (LPARAM)&mcs);
        if (!hwnd)
        {
            DWORD error = GetLastError();
            wchar_t errorMsg[256];
            MessageBox(NULL, errorMsg, L"Error", MB_ICONERROR);
        }

        //LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
        //style &= ~WS_CAPTION;
        //style &= ~WS_BORDER;
        //SetWindowLongPtr(hwnd, GWL_STYLE, style);
    }

    void HierarchyWindow::resize(const int newWidth, const int newHeight) noexcept
    {
        int width = newWidth * WIDTH_PROPORTION;
        int height = newHeight * HEIGHT_PROPORTION;

        MoveWindow(hwnd, newWidth - width, 0, width, height, TRUE);
    }

    LRESULT HierarchyWindow::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_CREATE:
            return 0;

        case WM_DESTROY:
            return 0;

        default:
            return DefMDIChildProc(hwnd, message, wParam, lParam);
        }
    }
};
