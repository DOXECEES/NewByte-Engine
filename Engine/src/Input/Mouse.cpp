#include "Mouse.hpp"

namespace nb
{
    namespace Input
    {


        bool Mouse::isRightClick() const noexcept
        {
            return (buttonFlags & RIGHT_BUTTON_DOWN) && !(prevButtonsFlag & RIGHT_BUTTON_DOWN);
        }

        bool Mouse::isLeftClick() const noexcept
        {
            return (buttonFlags & LEFT_BUTTON_DOWN) && !(prevButtonsFlag & LEFT_BUTTON_DOWN);
        }

        bool Mouse::isMiddleClick() const noexcept
        {
            return (buttonFlags & MIDDLE_BUTTON_DOWN) && !(prevButtonsFlag & MIDDLE_BUTTON_DOWN);
        }

        bool Mouse::isRightHeld() const noexcept
        {
            return (buttonFlags & RIGHT_BUTTON_DOWN);
        }

        bool Mouse::isLeftHeld() const noexcept
        {
            return (buttonFlags & LEFT_BUTTON_DOWN);
        }

        bool Mouse::isMiddleHeld() const noexcept
        {
            return (buttonFlags & MIDDLE_BUTTON_DOWN);
        }

        // чуствительность с конфигов
        void Mouse::update(const int x, const int y, const float scrollDelta, const uint16_t flags) noexcept
        {
            this->scrollDelta = scrollDelta;
            prevButtonsFlag = buttonFlags;
            buttonFlags = flags;

            const int deltaX = x - this->x;
            const int deltaY = y - this->y;

            this->x = x;
            this->y = y;

            yaw += deltaX * 0.1f;
            pitch += deltaY * 0.1f;

            if(pitch >  89.0f)
                pitch = 89.0f;
            if(pitch < -89.0f)
                pitch = -89.0f;
        
        }
    
        Math::Vector2<int> Mouse::getMousePosition() const noexcept
        {
            return {x, y};
        }

        Math::Vector2<float> Mouse::getPitchAndYaw() const noexcept
        {
            return {pitch, yaw};
        }

    };

    
};