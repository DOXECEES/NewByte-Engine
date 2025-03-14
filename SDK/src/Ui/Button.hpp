#ifndef SRC_UI_BUTTON_HPP
#define SRC_UI_BUTTON_HPP

#include <Windows.h>
#include <exception>
#include <stdexcept>

#include "IWidget.hpp"

namespace Ui
{
    class Button : public IWidget
    {
   
    public:
        Button(const Rect& rect, const std::wstring& text, std::function<void()> handler = 0, HWND parent = nullptr)
            : IWidget(rect, text, parent)
        {
            handle = CreateWindowW(L"BUTTON", text.c_str(), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                    rect.x, rect.y, rect.width, rect.height, parent, reinterpret_cast<HMENU>(id), nullptr, nullptr);

            onClickFunction = handler;
            IWidget::registrate();
        }

        void draw() override
        {
                        
        }



    private:

    };
};

#endif