#ifndef SRC_UTILS_DEFLATEDECODER_HPP
#define SRC_UTILS_DEFLATEDECODER_HPP

#include <bitset>
#include <array>
#include <vector>
#include <algorithm>
#include <iterator>
#include <unordered_map>


#include "BitReader.hpp"

namespace nb
{
    namespace Utils
    {
        class DeflateDecoder
        {


            static constexpr uint8_t MAX_COMMAND_ALPHABET_SIZE = 19;
            static constexpr std::array<uint8_t, MAX_COMMAND_ALPHABET_SIZE> COMMAND_ALPHABET_SEQUENCE = {
                16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
            };

            struct ZlibHeader
            {
                std::bitset<4> cm;
                std::bitset<4> cinfo;
                std::bitset<5> fcheak;
                std::bitset<1> fdict;
                std::bitset<2> flevel;
            };

        public:
            DeflateDecoder() = delete;
            DeflateDecoder(const std::vector<char> &vec);

            ZlibHeader readZlibHeader();
            void processDynamicHuffman();

        private:
            BitReader<char> br;
        };
    };
};

#endif