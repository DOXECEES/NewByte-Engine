#include "Keyboard.hpp"
#include "../Debug.hpp"
namespace nb
{
    namespace Input
    {
        void Keyboard::update() noexcept
        {
            prevFrameCodes = codes;
        }

        void Keyboard::setKeyUp(const uint8_t key) noexcept
        {
            codes[key] = false;
        }

        void Keyboard::setKeyDown(const uint8_t key) noexcept
        {
            codes[key] = true;
        }

        bool Keyboard::isKeyPressed(const uint8_t key) const noexcept
        {
            return (codes[key] && !prevFrameCodes[key]);
        }

        bool Keyboard::isKeyReleased(const uint8_t key) const noexcept
        {
            return (!codes[key] && prevFrameCodes[key]);
        }

        bool Keyboard::isKeyHeld(const uint8_t key) const noexcept
        {
            return codes[key];
        }
    };
};
