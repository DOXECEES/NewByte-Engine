#ifndef SRC_SHARED_HUFFMANTREE_HPP
#define SRC_SHARED_HUFFMANTREE_HPP

#include <memory>
#include <unordered_map>
#include <map>

#include "../Templates/BinaryTree.hpp"
#include "../Utils/BitReader.hpp"

#include "../Debug.hpp"

namespace nb
{
    namespace Shared
    {

        struct HuffmanNode
        {
            explicit HuffmanNode(std::string_view data, const uint32_t freq)
                : data(data), frequancy(freq)
            {
            }

            uint32_t frequancy = 0;
            std::string data = "";
            std::shared_ptr<HuffmanNode> left = nullptr;
            std::shared_ptr<HuffmanNode> right = nullptr;
        };

        class HuffmanTree : nb::Templates::BinaryTree<char, nb::Shared::HuffmanNode>
        {
        public:
            static constexpr uint16_t NO_CODE = 0xFFFF;

            HuffmanTree() = default;

            void insert(std::string_view data, uint32_t val = 0)
            {
            }

            std::map<uint16_t,uint16_t> buildCanonicalCodesByLength(std::vector<std::pair<uint16_t, uint16_t>> &commandAlphabet)
            {
                commandAlphabet.erase(std::remove_if(
                              commandAlphabet.begin(), commandAlphabet.end(),
                              [](const std::pair<uint16_t, uint16_t> &x)
                              {
                                  return x.second == 0;
                              }),
                          commandAlphabet.end());

                std::sort(commandAlphabet.begin(), commandAlphabet.end(), [](const std::pair<uint16_t, uint16_t> &a, const std::pair<uint16_t, uint16_t> &b) -> bool
                        { return a.second < b.second; });

                std::map<uint16_t, uint16_t> codes;
                uint16_t code = 0;
                uint16_t prevLength = 0;

                for (auto &i : commandAlphabet)
                {
                    if (i.second > prevLength)
                    {
                        code = code << (i.second - prevLength);
                        prevLength = i.second;
                    }

                    codes[code] = i.first;

                    std::string binaryCode = std::bitset<8>(code).to_string();
                    Debug::debug(binaryCode.c_str());
                    code += 1;
                }

                return codes;
            }

            constexpr void build(const std::vector<char> &vec)
            {
                // std::priority_queue<>

                for (const auto &i : vec)
                {
                }
            }

            void decode()
            {
            }

            // The maximum length of a Huffman code in PNG is 15 bits.
            // 0xFFFF - returns while no code find
            uint16_t decodeCanonical(Utils::BitReader<char> &br,
                                     const std::map<uint16_t, uint16_t> &codeToId,
                                     const std::vector<std::pair<uint16_t, uint16_t>> &idToLength)
            {
                uint16_t code = 0;

                // код - id
                // id - длина

                std::unordered_map<uint16_t, uint16_t> codeLengthMap;
                auto codesBegin = codeToId.begin();
                for (int i = 0; i < idToLength.size(); i++)
                {
                    codeLengthMap[codesBegin->first] = idToLength[i].second;
                    codesBegin++;
                }

                for (int i = 0; i < 16; i++)
                {
                    code <<= 1;
                    code |= br.readRightToLeft(1);
                    if (codeLengthMap.contains(code) && codeLengthMap[code] == i + 1)
                    {
                        return code;
                    }
                }

                return NO_CODE; 
            }

        private:
        };
    };
};

#endif