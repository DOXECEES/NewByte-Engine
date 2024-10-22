#ifndef SRC_FATAL_HPP
#define SRC_FATAL_HPP

#include <Windows.h>

#include "Window.hpp"

namespace nb
{
    class Fatal
    {
    public:
    
        inline static void init(Window* wnd) noexcept
        {
            wind = wnd;
        }

        inline static void exit(LPCWSTR text) noexcept
        {
            if(wind != nullptr)
                wind->exit(text);
        }

    private:

        static Window* wind;
    };
};


#endif
