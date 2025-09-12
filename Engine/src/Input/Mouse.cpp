// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "Mouse.hpp"

#include "../Debug.hpp"

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

        void Mouse::update(const int x, const int y, const float scrollDelta, const uint16_t flags) noexcept
        {
            this->scrollDelta = scrollDelta;
           
            prevButtonsFlag = buttonFlags;
            buttonFlags = flags;
            
            deltaX = x - this->x;
            deltaY = y - this->y;

            this->x = x;
            this->y = y;

            const float mouseSensivity = nb::Core::EngineSettings::getMouseSensivity();

            yaw += deltaX * mouseSensivity;
            pitch += deltaY * mouseSensivity;

            if(pitch >  POSITIVE_PITCH_LIMIT)
                pitch = POSITIVE_PITCH_LIMIT;
            if(pitch < NEGATIVE_PITCH_LIMIT)
                pitch = NEGATIVE_PITCH_LIMIT;
        
        }
    
        Math::Vector2<int> Mouse::getMousePosition() const noexcept
        {
            return {x, y};
        }

        Math::Vector2<int> Mouse::getDeltaPosition() const noexcept
        {
            return {deltaX, deltaY};
        }

        Math::Vector2<float> Mouse::getPitchAndYaw() const noexcept
        {
            return {pitch, yaw};
        }

    };

    
};