#ifndef SRC_UI_UIMAPPER_HPP
#define SRC_UI_UIMAPPER_HPP

#include <Windows.h>

#include <unordered_map>

#include "IWidget.hpp"

namespace Ui
{
    class UiMapper
    {
    public:
        //static void add(const HWND hwnd, IWidget *widget);
        
        //static IWidget *get(const HWND hwnd);

    private:
        //inline static std::unordered_map<HWND, IWidget*> mapper = {};
    };
};

#endif