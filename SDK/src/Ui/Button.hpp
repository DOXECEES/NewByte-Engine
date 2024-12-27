#ifndef SRC_UI_BUTTON_HPP
#define SRC_UI_BUTTON_HPP

#include <Windows.h>
#include <functional>
#include <exception>
#include <stdexcept>



namespace Ui
{
    class Button
    {
   
    public:
        Button(HWND parent, const std::wstring &text, int x, int y, int width, int height, std::function<void()> onClick)
            : onClick(onClick)
        {
            hWndButton = CreateWindow(
                L"BUTTON", text.c_str(),
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                x, y, width, height,
                parent, nullptr, GetModuleHandle(nullptr), nullptr);

            if (!hWndButton)
            {
                throw std::runtime_error("Failed to create button.");
            }

            if (SetWindowLongPtr(hWndButton, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this)))
            {
                throw std::runtime_error("Failed to set GWLP_USERDATA.");
            }

            originalWndProc = reinterpret_cast<WNDPROC>(
                SetWindowLongPtr(hWndButton, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(ButtonWndProc)));

            if (!originalWndProc)
            {
                throw std::runtime_error("Failed to subclass button.");
            }
        }

        ~Button()
        {
            if (hWndButton)
            {
                SetWindowLongPtr(hWndButton, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(originalWndProc));
            }
        }

        HWND GetHandle() const
        {
            return hWndButton;
        }

    private:
        HWND hWndButton;
        WNDPROC originalWndProc;
        std::function<void()> onClick;

        static LRESULT CALLBACK ButtonWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
        {
            Button *button = reinterpret_cast<Button *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

            if (button)
            {
                switch (msg)
                {
                case WM_LBUTTONDOWN:
                    if (button->onClick)
                    {
                        button->onClick();
                    }
                    return 0;
                default:
                    break;
                }
            }

            return CallWindowProc(button->originalWndProc, hWnd, msg, wParam, lParam);
        }

    };

}

#endif