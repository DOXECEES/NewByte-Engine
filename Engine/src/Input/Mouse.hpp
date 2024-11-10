#ifndef SRC_INPUT_MOUSE_HPP
#define SRC_INPUT_MOUSE_HPP

#include "../Math/Vector2.hpp"

namespace nb
{
    namespace Input
    {
        class Mouse
        {
            // copied from winapi struct;
            enum ButtonCodes
            {
                LEFT_BUTTON_DOWN = 0x0001,
                LEFT_BUTTON_UP = 0x0002,
                RIGHT_BUTTON_DOWN = 0x0004,
                RIGHT_BUTTON_UP = 0x0008,
                MIDDLE_BUTTON_DOWN = 0x0010,
                MIDDLE_BUTTON_UP = 0x0020,
                BUTTON_4_DOWN = 0x0040,
                BUTTON_4_UP = 0x0080,
                BUTTON_5_DOWN = 0x0100,
                BUTTON_5_UP = 0x0200
            };

        public:
            Mouse() = default;
            ~Mouse() = default;

            bool isRightClick() const noexcept;
            bool isLeftClick() const noexcept;
            bool isMiddleClick() const noexcept;

            bool isRightHeld() const noexcept;
            bool isLeftHeld() const noexcept;
            bool isMiddleHeld() const noexcept;

            void update(const int x, const int y, const float scrollDelta ,const uint16_t flags) noexcept;

            inline int getX() const noexcept { return x; };
            inline int getY() const noexcept { return y; };

            Math::Vector2<int> getMousePosition() const noexcept;

            inline float getYaw() const noexcept { return yaw; };
            inline float getPitch() const noexcept { return pitch; };

            Math::Vector2<float> getPitchAndYaw() const noexcept;
            inline float getScrollDelta() const noexcept { return scrollDelta; };

        private:

            int x           = 0;
            int y           = 0;

            float scrollDelta = 0.0f;
            uint16_t prevButtonsFlag = 0;
            uint16_t buttonFlags = 0;

            float pitch     = 0.0f;
            float yaw       = 0.0f;
        
        };
    };
};

#endif


