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
                , dataSizeInBytes(vec.size() * sizeof(DataType))
            {
            }

            std::optional<bool> getBitLs()
            {
                bool res = data[currentBytePointer] & (1 << lsBitPointer);

                lsBitPointer++;

                if(lsBitPointer + msBitPointer == 8)
                {
                    msBitPointer = 0;
                    lsBitPointer = 0;
                    currentBytePointer++;
                }

                if(isOutOfRange())
                {
                    return std::nullopt;
                }

                return res;
            }

            std::optional<bool> getBitMs()
            {
                bool res = (data[currentBytePointer] & (1 << (8 - msBitPointer - 1)));

                msBitPointer++;

                if(lsBitPointer + msBitPointer == 8)
                {
                    msBitPointer = 0;
                    lsBitPointer = 0;
                    currentBytePointer++;
                }

                if(isOutOfRange())
                {
                    return std::nullopt;
                }
             
                return res;
            }

            std::optional<bool> peekBitLs()
            {
                if (isOutOfRange())
                {
                    return std::nullopt;
                }

                return (data[currentBytePointer] & (1 << lsBitPointer));
            }

            std::optional<bool> peekBitMs()
            {
                if (isOutOfRange())
                {
                    return std::nullopt;
                }
                return (data[currentBytePointer] & (1 << (8 - msBitPointer - 1)));
            }

            std::optional<bool> peekBitLsLookAhead(size_t offset)
            {
                size_t copyBytesPos = currentBytePointer;
                uint8_t copyLsBitPointer = lsBitPointer;
                uint8_t copyMsBitPointer = msBitPointer;

                while(offset > 0)
                {
                    copyLsBitPointer++;
                    offset--;
                    if(copyLsBitPointer + copyMsBitPointer == 8)
                    {
                        copyMsBitPointer = 0;
                        copyLsBitPointer = 0;
                        copyBytesPos++;
                    }
                }

                return (data[copyBytesPos] & (1 << copyLsBitPointer));
            }

            std::optional<bool> peekBitMsLookAhead(size_t offset)
            {
                size_t copyBytesPos = currentBytePointer;
                uint8_t copyLsBitPointer = lsBitPointer;
                uint8_t copyMsBitPointer = msBitPointer;

                while(offset > 0)
                {
                    copyMsBitPointer++;
                    offset--;
                    if(copyLsBitPointer + copyMsBitPointer == 8)
                    {
                        copyMsBitPointer = 0;
                        copyLsBitPointer = 0;
                        copyBytesPos++;
                    }

                }

                return (data[copyBytesPos] & (1 << (8 - copyMsBitPointer - 1)));
            }

            template <typename T = uint32_t>
            typename std::enable_if<std::is_integral<T>::value, T>::type
            readLeftToRight(const uint8_t count)
            {
                T res = 0;
                for (uint8_t i = 0; i < count; i++)
                {
                    auto bit = getBitMs(); 
                    if (bit.has_value())
                        res |= (*bit << i);
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
                    auto bit = getBitLs(); 
                    if (bit.has_value())
                    {
                        res <<= i;
                        res |= (static_cast<uint32_t>(bit.value()) << i);
                    }
                }

                return res;
            }

            constexpr void setBytePosition(const size_t pos) noexcept
            {
                currentBytePointer = pos;
            }

            constexpr void setLsBitPointer(const uint8_t pos) noexcept
            {
                lsBitPointer = pos;
            }

            constexpr void setMsBitPointer(const uint8_t pos) noexcept
            {
                msBitPointer = pos;
            }

            constexpr size_t getBytePointer() const noexcept
            {
                return currentBytePointer;
            }

            constexpr uint8_t getLsBitPointer() const noexcept
            {
                return lsBitPointer;
            }

            constexpr uint8_t getMsBitPointer() const noexcept
            {
                return msBitPointer;
            }

            constexpr bool isOutOfRange() const noexcept
            {
                return (currentBytePointer >= dataSizeInBytes);
            }
      

        private:


            size_t currentBytePointer       = 0;
            uint8_t lsBitPointer             = 0;
            uint8_t msBitPointer             = 0;

            size_t dataSizeInBytes          = 0;

            const DataType *data            = nullptr;
        };
    };
};
#endif