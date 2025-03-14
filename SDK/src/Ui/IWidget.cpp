#include "IWidget.hpp"

namespace Ui 
{
    IWidget::IWidget(const Rect &rect, const std::wstring &text, HWND parent)
        : rect(rect)
        , text(text)
        , id(globalId++)
        , parentHandle(parent)
    {
        
    }

    void IWidget::registrate() noexcept
    {
        //UiMapper::add(handle, this);
    }
};
