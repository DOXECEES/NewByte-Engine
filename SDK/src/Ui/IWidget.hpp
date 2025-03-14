#ifndef SRC_UI_IWIDGET_HPP
#define SRC_UI_IWIDGET_HPP

#include <Windows.h>

#include <string>
#include <functional>
#include <stdexcept>

#include "UiMapper.hpp"



namespace Ui
{
    struct Rect
    {
        int x;
        int y;
        int width;
        int height;

        Rect(const int x, const int y, const int width, const int height)
            : x(x)
            , y(y)
            , width(width)
            , height(height)
        {}

    };

    class IWidget
    {
    public:
        IWidget(const Rect& rect, const std::wstring& text, HWND parent = nullptr);
        

        virtual ~IWidget() = default;
        virtual void draw() = 0;

        virtual void onClick() = 0;

        void setOnClickFunction(std::function<void()> function)
        {
            onClickFunction = function;
        }

    
    protected:

        // Создание всех наследников должно завершатся вызовом registrate
        void registrate() noexcept;

        Rect rect;
        std::wstring text;
        HWND parentHandle;
        HWND handle;

        std::function<void()> onClickFunction = []()
        { 
            throw std::runtime_error("Button has no onClick function");
        };

        int id;

        
    private:    

        inline static int globalId = 0;
    
    };

};

#endif



