#include "BitReader.hpp"

// template <typename DataType>
// nb::Utils::BitReader<DataType>::BitReader(const std::vector<DataType> &vec)
//     : data(vec.data()), dataSize(vec.size())
// {
// }

// template <typename DataType>
// template <typename T>
// typename std::enable_if<std::is_integral<T>::value, std::optional<T>>::type nb::Utils::BitReader<DataType>::getBit()
// {
    
// }

// template <typename DataType>
// template <typename T>
// typename std::enable_if<std::is_integral<T>::value, std::optional<T>>::type
// nb::Utils::BitReader<DataType>::peekBit()
// {
    
// }

// template <typename DataType>
// template <typename T>
// typename std::enable_if<std::is_integral<T>::value, T>::type nb::Utils::BitReader<DataType>::readLeftToRight(const uint8_t count)
// {
//     return typename std::enable_if<std::is_integral<T>::value, T>::type();
// }

// template <typename DataType>
// template <typename T>
// typename std::enable_if<std::is_integral<T>::value, T>::type
// nb::Utils::BitReader<DataType>::readRightToLeft(const uint8_t count)
// {
//     return typename std::enable_if<std::is_integral<T>::value, T>::type();
// }

// uint32_t nb::Loaders::BitReader::readLeftToRight(const uint8_t count)
// {
//     uint32_t res = 0;

//     for (size_t i = 0; i < count; i++)
//     {
//         res |= (peekBit() << (count - i - 1));
//     }

//     return res;
// }

// uint32_t nb::Loaders::BitReader::readRightToLeft(const uint8_t count)
// {
//     uint32_t res = 0;

//     for (size_t i = 0; i < count; i++)
//     {
//         res |= (peekBit() << i);
//     }

//     return res;
// }
