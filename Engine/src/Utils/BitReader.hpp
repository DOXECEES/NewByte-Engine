#ifndef SRC_UTILS_BITREADER_HPP
#define SRC_UTILS_BITREADER_HPP

#include <cstdint>
#include <vector>
#include <optional>

namespace nb
{
    namespace Utils
    {
        template <typename DataType>
        class BitReader
        {
        public:
            BitReader() = delete;
            BitReader(const std::vector<DataType> &vec)
                : data(vec.data())
                , dataSizeInBits(vec.size() * sizeof(DataType) * 8)
            {
            }

            std::optional<bool> getBit()
            {
                currentPointerPos++;

                if (isOutOfRange())
                {
                    return std::nullopt;
                }

                return (data[(currentPointerPos-1) / 8] & (1 << ((currentPointerPos-1) % 8)));
            }

            
            std::optional<bool> peekBit()
            {
                if (isOutOfRange())
                {
                    return std::nullopt;
                }

                return (data[(currentPointerPos) / 8] & (1 << ((currentPointerPos) % 8)));
            }

            template <typename T = uint32_t>
            typename std::enable_if<std::is_integral<T>::value, T>::type
            readLeftToRight(const uint8_t count)
            {
                T res = 0;
                for (uint8_t i = 0; i < count; i++)
                {
                    auto bit = getBit(); 
                    if (bit.has_value())
                        res |= (bit.value() << (count - i - 1));
                }

                return res;
            }

            template <typename T = uint32_t>
            typename std::enable_if<std::is_integral<T>::value, T>::type
            readRightToLeft(const uint8_t count)
            {
                T res = 0;
                for (size_t i = 0; i < count; i++)
                {
                    auto bit = getBit(); 
                    if (bit.has_value())
                        res |= (bit.value() << i);
                }

                return res;
            }

            constexpr void setPosition(const size_t pos) noexcept
            {
                currentPointerPos = pos;
            }

            constexpr auto getPosition() const noexcept
            {
                return currentPointerPos;
            }

            constexpr bool isOutOfRange() const noexcept
            {
                return (currentPointerPos >= dataSizeInBits);
            }
        private:


            size_t currentPointerPos = 0;
            size_t dataSizeInBits = 0;

            const DataType *data = nullptr;
        };
    };
};
#endif