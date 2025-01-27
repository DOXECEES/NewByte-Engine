#ifndef SRC_INPUT_INPUT_HPP
#define SRC_INPUT_INPUT_HPP

#include <Windows.h>

#include "Keyboard.hpp"
#include "Mouse.hpp"

#include "../Core.hpp"

namespace nb
{
    namespace Input
    {
        class Input
        {
        public:
            Input();
            ~Input() = default;

            void linkMouse(Ref<Mouse> mouse) noexcept;
            void linkKeyboard(Ref<Keyboard> keyboard) noexcept;

            inline void stopHandlingPosition() noexcept { shouldHandlePosition = false; };
            inline void startHandlingPosition() noexcept { shouldHandlePosition = true; };
            inline void stopHandlingKeyboardEvents() noexcept { shouldHandleKeyboardEvents = false; };
            inline void startHandlingKeyboardEvents() noexcept { shouldHandleKeyboardEvents = true; };
            
            void update(const MSG& msg) noexcept;
            void updateAll() const noexcept;


        private:
            void reset() noexcept;

        private:
            bool shouldHandlePosition = false;
            bool shouldHandleKeyboardEvents = true;

            uint16_t deviceFlags;

            float prevScrollData = 0.0f;
            int prevMouseX          = 0;
            int prevMouseY          = 0;
        
            Ref<Mouse> mouse        = nullptr;
            Ref<Keyboard> keyboard  = nullptr;


        };
    };
};

#endif

