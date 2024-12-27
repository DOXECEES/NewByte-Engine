#ifndef SRC_UI_RADIOGROUP_HPP
#define SRC_UI_RADIOGROUP_HPP

#include <Windows.h>

#include <map>
#include <string>
#include <functional>

namespace Ui
{
    class RadioGroup
    {
    public:
        RadioGroup(HWND parent, const std::wstring &groupName, const std::map<std::wstring, std::function<void()>> &mapper)
            : groupBoxHwnd(nullptr)
        {
            groupBoxHwnd = CreateWindow(
                L"BUTTON", groupName.c_str(),
                WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
                10, 300, 200, 200,
                parent, nullptr, GetModuleHandle(nullptr), nullptr);

            const int xOffset = 20;
            const int yOffset = 30;
            int buttonCount = 0;

            for (auto &i : mapper)
            {
                DWORD style = WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON;
                if (buttonCount == 0)
                {
                    style |= WS_GROUP;
                    style |= WS_TABSTOP;
                }

                HWND radio = CreateWindow(
                    L"BUTTON", i.first.c_str(),
                    style,
                    20, 330 + yOffset * buttonCount, 160, 20,
                    parent, reinterpret_cast<HMENU>(sId++), GetModuleHandle(nullptr), nullptr);

                SetWindowLongPtr(radio, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&i.second));

                radioButtons[radio] = i.second;
                buttonCount++;
            }
        }

        void handle(UINT message, WPARAM wParam, LPARAM lParam)
        {
            switch (message)
            {
            case WM_COMMAND:
            {

                if (HIWORD(wParam) == BN_CLICKED)
                {
                    int controlId = LOWORD(wParam);
                    auto it = radioButtons.find((HWND)lParam);
                    if (it != radioButtons.end())
                    {
                        it->second();
                    }
                }
                break;
            }
            default:
                break;
            }
        }

    private:
        HWND groupBoxHwnd;
        std::map<HWND, std::function<void()>> radioButtons;
        static int sId;
    };
};

#endif