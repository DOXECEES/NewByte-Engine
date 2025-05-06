#ifndef SRC_LOADERS_PNGLOADER_HPP
#define SRC_LOADERS_PNGLOADER_HPP

#include <stdint.h>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <array>
#include <bitset>

#include "../Debug.hpp"
#include "../Core.hpp"
#include "../Utils/BitReader.hpp"

namespace nb
{
    namespace Loaders
    {

        class PngLoader
        {
            static constexpr uint8_t PNG_HEADER_SIZE = 8;
            static constexpr std::array<uint8_t, PNG_HEADER_SIZE> PNG_HEADER = {
                0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A
            };
            

            static constexpr uint8_t CHUNK_SIZE_IN_BYTES        = 4;
            static constexpr uint8_t CHUNK_TYPE_SIZE_IN_BYTES   = 4;
            static constexpr uint8_t CHUNK_CRC_SIZE_IN_BYTES    = 4;

            struct ChunkData
            {
                char name[4];
                std::vector<char> data;
                uint32_t crc;
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

            void identifyChunk(const ChunkData &chunk);
            void readChunk();

            std::ifstream file;
            bool isGood = true;
        };
    };

};





#endif