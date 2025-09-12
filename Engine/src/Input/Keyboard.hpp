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

			enum class KeyCode : uint8_t 
			{
				NB_BACKSPACE = 0x08,
				NB_TAB = 0x09,
				NB_RETURN = 0x0D,
				NB_SHIFT = 0x10,
				NB_CONTROL = 0x11,
				NB_ALT = 0x12,
				NB_PAUSE = 0x13,
				NB_CAPSLOCK = 0x14,
				NB_ESCAPE = 0x1B,
				NB_SPACE = 0x20,
				NB_PAGEUP = 0x21,
				NB_PAGEDOWN = 0x22,
				NB_END = 0x23,
				NB_HOME = 0x24,
				NB_LEFT = 0x25,
				NB_UP = 0x26,
				NB_RIGHT = 0x27,
				NB_DOWN = 0x28,
				NB_PRINTSCREEN = 0x2C,
				NB_INSERT = 0x2D,
				NB_DELETE = 0x2E,

				// Numbers 0-9
				NB_0 = 0x30,
				NB_1 = 0x31,
				NB_2 = 0x32,
				NB_3 = 0x33,
				NB_4 = 0x34,
				NB_5 = 0x35,
				NB_6 = 0x36,
				NB_7 = 0x37,
				NB_8 = 0x38,
				NB_9 = 0x39,

				// Letters A-Z
				NB_A = 0x41,
				NB_B = 0x42,
				NB_C = 0x43,
				NB_D = 0x44,
				NB_E = 0x45,
				NB_F = 0x46,
				NB_G = 0x47,
				NB_H = 0x48,
				NB_I = 0x49,
				NB_J = 0x4A,
				NB_K = 0x4B,
				NB_L = 0x4C,
				NB_M = 0x4D,
				NB_N = 0x4E,
				NB_O = 0x4F,
				NB_P = 0x50,
				NB_Q = 0x51,
				NB_R = 0x52,
				NB_S = 0x53,
				NB_T = 0x54,
				NB_U = 0x55,
				NB_V = 0x56,
				NB_W = 0x57,
				NB_X = 0x58,
				NB_Y = 0x59,
				NB_Z = 0x5A,

				// Numpad keys
				NB_NUMPAD0 = 0x60,
				NB_NUMPAD1 = 0x61,
				NB_NUMPAD2 = 0x62,
				NB_NUMPAD3 = 0x63,
				NB_NUMPAD4 = 0x64,
				NB_NUMPAD5 = 0x65,
				NB_NUMPAD6 = 0x66,
				NB_NUMPAD7 = 0x67,
				NB_NUMPAD8 = 0x68,
				NB_NUMPAD9 = 0x69,
				NB_MULTIPLY = 0x6A,
				NB_ADD = 0x6B,
				NB_SEPARATOR = 0x6C,
				NB_SUBTRACT = 0x6D,
				NB_DECIMAL = 0x6E,
				NB_DIVIDE = 0x6F,

				// Function keys
				NB_F1 = 0x70,
				NB_F2 = 0x71,
				NB_F3 = 0x72,
				NB_F4 = 0x73,
				NB_F5 = 0x74,
				NB_F6 = 0x75,
				NB_F7 = 0x76,
				NB_F8 = 0x77,
				NB_F9 = 0x78,
				NB_F10 = 0x79,
				NB_F11 = 0x7A,
				NB_F12 = 0x7B,
				NB_F13 = 0x7C,
				NB_F14 = 0x7D,
				NB_F15 = 0x7E,
				NB_F16 = 0x7F,
				NB_F17 = 0x80,
				NB_F18 = 0x81,
				NB_F19 = 0x82,
				NB_F20 = 0x83,
				NB_F21 = 0x84,
				NB_F22 = 0x85,
				NB_F23 = 0x86,
				NB_F24 = 0x87,

				// Other keys
				NB_NUMLOCK = 0x90,
				NB_SCROLLLOCK = 0x91,

				// Modifier keys
				NB_LSHIFT = 0xA0,
				NB_RSHIFT = 0xA1,
				NB_LCONTROL = 0xA2,
				NB_RCONTROL = 0xA3,
				NB_LALT = 0xA4,
				NB_RALT = 0xA5,

				// OEM keys
				NB_SEMICOLON = 0xBA,   // ;:
				NB_EQUALS = 0xBB,   // =+
				NB_COMMA = 0xBC,   // ,<
				NB_MINUS = 0xBD,   // -_
				NB_PERIOD = 0xBE,   // .>
				NB_SLASH = 0xBF,   // /?
				NB_GRAVE = 0xC0,   // `~
				NB_LBRACKET = 0xDB,   // [{
				NB_BACKSLASH = 0xDC,   // \|
				NB_RBRACKET = 0xDD,   // ]}
				NB_QUOTE = 0xDE    // '"
			};

            Keyboard() noexcept = default;
            ~Keyboard() noexcept = default;

            void update() noexcept;

            void setKeyDown(const uint8_t key) noexcept;
            void setKeyUp(const uint8_t key) noexcept;

            bool isKeyPressed(const KeyCode key) const noexcept;
            bool isKeyReleased(const KeyCode key) const noexcept;

            bool isKeyHeld(const KeyCode key) const noexcept;
            

        private:
            std::bitset<MAX_COUNT_OF_VK> codes;
            std::bitset<MAX_COUNT_OF_VK> prevFrameCodes = codes;
        };
    };
};

#endif

