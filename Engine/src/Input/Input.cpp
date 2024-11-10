#include "Input.hpp"

#include "../Debug.hpp"
#include "../Core/Engine.hpp"

namespace nb
{
    namespace Input
    {
        Input::Input()
        {
            // 0 - mouse 
            // 1 - keyboard
            RAWINPUTDEVICE rawDevice[2];

            rawDevice[0].usUsagePage  = 0x01;
            rawDevice[0].usUsage      = 0x02;
            rawDevice[0].dwFlags      = 0x00;
            rawDevice[0].hwndTarget   = nb::Core::Engine::getLinkedHwnd();

            rawDevice[1].usUsagePage  = 0x01;
            rawDevice[1].usUsage      = 0x06;
            rawDevice[1].dwFlags      = RIDEV_NOLEGACY;
            rawDevice[1].hwndTarget   = nb::Core::Engine::getLinkedHwnd();

            RegisterRawInputDevices(rawDevice, 2, sizeof(RAWINPUTDEVICE));
        }

        void Input::linkMouse(Ref<Mouse> mouse) noexcept
        {
            this->mouse = mouse;
        }

        void Input::linkKeyboard(Ref<Keyboard> keyboard) noexcept
        {
            this->keyboard = keyboard;
        }

        void Input::update(const MSG& msg) noexcept
        {
            if(msg.message == WM_INPUT)
            {
                uint32_t size = sizeof(RAWINPUT);
                static RAWINPUT rawInput;
                auto i = GetRawInputData(reinterpret_cast<HRAWINPUT>(msg.lParam), RID_INPUT, &rawInput, &size, sizeof(RAWINPUTHEADER));

                switch (rawInput.header.dwType)
                {
                case RIM_TYPEMOUSE:
                {
                    if(rawInput.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)
                    {
                        /** 
                        *   @todo Обработка мыши при абсолючтной позиции
                        */


                        prevMouseX = rawInput.data.mouse.lLastX;
                        prevMouseY = rawInput.data.mouse.lLastY;
                    }
                    else if(rawInput.data.mouse.lLastX != 0 || rawInput.data.mouse.lLastY != 0)
                    {
                        prevMouseX += rawInput.data.mouse.lLastX;
                        prevMouseY += rawInput.data.mouse.lLastY;
                    }

                    if((rawInput.data.mouse.usButtonFlags & RI_MOUSE_WHEEL) 
                        || rawInput.data.mouse.usButtonFlags & RI_MOUSE_HWHEEL)
                    {
                        int16_t wheelDelta = static_cast<int16_t>(rawInput.data.mouse.usButtonData);
                        prevScrollData = static_cast<float>(wheelDelta) / WHEEL_DELTA;

                        if (rawInput.data.mouse.usButtonFlags & RI_MOUSE_HWHEEL) 
                        {
                            uint64_t scrollChars = 1; // 1 is the default
                            SystemParametersInfo(SPI_GETWHEELSCROLLCHARS, 0, &scrollChars, 0);
                            prevScrollData *= scrollChars;
                            
                        }
                        else 
                        {
                            uint64_t scrollLines = 3; 
                            SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &scrollLines, 0);
                            if (scrollLines != WHEEL_PAGESCROLL)
                                prevScrollData *= scrollLines;
                            
                        }
                    }
                    
                    deviceFlags = rawInput.data.mouse.usButtonFlags;

                    break;
                }
                case RIM_TYPEKEYBOARD:
                
                    //rawInput.data.keyboard
                    break;
                case RIM_TYPEHID:
                    //rawInput.data.hid
                    break;
                default:
                    break;
                }
            }
            else
            {
                reset();
            }

            mouse->update(prevMouseX, prevMouseY, prevScrollData, deviceFlags);

        }

        void Input::reset() noexcept
        {
            //prevMouseX = 0;
            //prevMouseY = 0;
            prevScrollData = 0;
            //deviceFlags = 0;
        }
    };
};