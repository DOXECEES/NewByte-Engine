// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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

        bool Keyboard::isKeyPressed(const KeyCode key) const noexcept
        {
            return (codes[static_cast<uint8_t>(key)] && !prevFrameCodes[static_cast<uint8_t>(key)]);
        }

        bool Keyboard::isKeyReleased(const KeyCode key) const noexcept
        {
            return (!codes[static_cast<uint8_t>(key)] && prevFrameCodes[static_cast<uint8_t>(key)]);
        }

        bool Keyboard::isKeyHeld(const KeyCode key) const noexcept
        {
            return codes[static_cast<uint8_t>(key)];
        }

    };
};
