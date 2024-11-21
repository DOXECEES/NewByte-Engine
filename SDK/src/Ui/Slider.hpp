#ifndef SRC_UI_SLIDER_HPP
#define SRC_UI_SLIDER_HPP

#include <Windows.h>
#include <commctrl.h> 

#pragma comment(lib, "Comctl32.lib") 

namespace Ui
{

    class Slider
    {
    public:
        Slider(HWND parent, int x, int y, int width, int height, int minVal, int maxVal, int initVal, int id)
            : hParent(parent), sliderId(id), minValue(minVal), maxValue(maxVal), value(initVal)
        {
            // Создание слайдера
            hSlider = CreateWindowEx(
                0, TRACKBAR_CLASS, nullptr,
                WS_CHILD | WS_VISIBLE | TBS_HORZ,
                x, y, width, height,
                parent, (HMENU)id, GetModuleHandle(nullptr), nullptr);

            if (!hSlider)
            {
                MessageBox(parent, L"Failed to create slider!", L"Error", MB_ICONERROR);
                return;
            }

            // Настройка диапазона и начального значения
            SendMessage(hSlider, TBM_SETRANGE, TRUE, MAKELPARAM(minVal, maxVal));
            SendMessage(hSlider, TBM_SETPOS, TRUE, initVal);
        }

        ~Slider()
        {
            if (hSlider)
                DestroyWindow(hSlider);
        }

        // Получить текущее значение
        int GetValue() const
        {
            return (int)SendMessage(hSlider, TBM_GETPOS, 0, 0);
        }

        // Установить значение
        void SetValue(int val)
        {
            if (val >= minValue && val <= maxValue)
            {
                SendMessage(hSlider, TBM_SETPOS, TRUE, val);
            }
        }

        // Установить диапазон
        void SetRange(int minVal, int maxVal)
        {
            minValue = minVal;
            maxValue = maxVal;
            SendMessage(hSlider, TBM_SETRANGE, TRUE, MAKELPARAM(minVal, maxVal));
        }

        // Получить идентификатор
        int GetId() const
        {
            return sliderId;
        }

        // Получить хендл окна слайдера
        HWND GetHandle() const
        {
            return hSlider;
        }

    private:
        HWND hSlider = nullptr;
        HWND hParent = nullptr;
        int sliderId;
        int minValue;
        int maxValue;
        int value;
    };
};

#endif

