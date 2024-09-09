#ifndef SRC_LOADERS_PNGLOADER_HPP
#define SRC_LOADERS_PNGLOADER_HPP

#include <stdint.h>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <array>
#include <bitset>

#include "../Debug.hpp"

namespace nb
{
    namespace Loaders
    {

class BitReader
        {
        
        public:

            BitReader() = default;
            BitReader(const std::vector<char> &vec);

            bool peekBit();

            uint32_t readLeftToRight(const uint8_t count);
            uint32_t readRightToLeft(const uint8_t count);

        private:

            size_t currentPointerPos = 0;
            size_t dataSize = 0;
            
            const char *data;
        };

        class PngLoader
        {
            static constexpr uint8_t PNG_HEADER_SIZE = 8;
            static constexpr std::array<uint8_t, PNG_HEADER_SIZE> PNG_HEADER = {
                0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A
            };
            static constexpr uint8_t MAX_COMMAND_ALPHABET_SIZE = 19;
            static constexpr std::array<uint8_t, MAX_COMMAND_ALPHABET_SIZE> COMMAND_ALPHABET_SEQUENCE = {
                16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
            };

            static constexpr uint8_t CHUNK_SIZE_IN_BYTES = 4;
            static constexpr uint8_t CHUNK_TYPE_SIZE_IN_BYTES = 4;
            static constexpr uint8_t CHUNK_CRC_SIZE_IN_BYTES = 4;

            struct ChunkData
            {
                char name[4];
                std::vector<char> data;
                uint32_t crc;
            };

            struct ZlibHeader
            {
                std::bitset<4> cm;
                std::bitset<4> cinfo;
                std::bitset<5> fcheak;
                std::bitset<1> fdict;
                std::bitset<2> flevel;
            };

            struct IHDR
            {
                uint32_t width = 0;
                uint32_t height = 0;
                uint8_t bitDepth = 0;
                uint8_t colorType = 0;
                uint8_t compressionMethod = 0;
                uint8_t filterMethod = 0;
                uint8_t interlaceMethod = 0;
            };


        public:
            PngLoader(const std::string &filePath);

            inline bool good() const { return isGood; };

            template<typename T>
            constexpr T swapEndian(T& val)
            {
                union Value
                {
                    T value;
                    uint8_t bits[sizeof(T)];
                } un;

                un.value = val;

                T res = 0;
                for (size_t i = 0; i < (sizeof(T)) ; i++)
                {
                    //Debug::debug(sizeof(T));
                    res |= static_cast<T>(un.bits[i]) << (8 * (sizeof(T) - 1 - i));
                }
                return res;
            }


        private:
            bool validateHeader();
            bool validateIhdr();

            ZlibHeader readZlibHeader(BitReader &br);

            void identifyChunk(const ChunkData &chunk);
            void readChunk();

            std::ifstream file;
            bool isGood = true;
        };
    
    
        
    };

};





#endif