#ifndef SRC_INPUT_KEYBOARD_HPP
#define SRC_INPUT_KEYBOARD_HPP

#include <bitset>

namespace nb
{
    namespace Input
    {

        class Keyboard
        {
            inline static constexpr uint8_t MAX_COUNT_OF_VK = 0xFF;

        public:
            Keyboard() noexcept = default;
            ~Keyboard() noexcept = default;

            void update() noexcept;

            void setKeyDown(const uint8_t key) noexcept;
            void setKeyUp(const uint8_t key) noexcept;

            bool isKeyPressed(const uint8_t key) const noexcept;
            bool isKeyReleased(const uint8_t key) const noexcept;

            bool isKeyHeld(const uint8_t key) const noexcept;
            


        private:
            std::bitset<MAX_COUNT_OF_VK> codes;
            std::bitset<MAX_COUNT_OF_VK> prevFrameCodes;
        };
    };
};

#endif

