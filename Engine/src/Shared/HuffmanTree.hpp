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

                    std::string binaryCode = std::bitset<16>(code).to_string();
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
            uint16_t decodeCanonical(Utils::BitReader<char>& br 
                , const std::map<uint16_t, uint16_t>& codes
                , const std::vector<std::pair<uint16_t, uint16_t>>& c)
            {
                uint16_t len = 0;
                uint16_t code = 0; 
                uint16_t prevCode = 0;

                std::map<uint16_t, uint16_t> l;

                for(const auto& i : c)
                {
                    l[i.first] = i.second;
                }

                // FUCK THIS CODES

                for (int length = 1; length <= 15; ++length)
                {
                            //code = (code << 1) | br.getBitLs().value();

                    //code |= (br.getBitMs().value() << length -1 );
                    code = (code << 1) | br.getBitMs().value();

                    //uint16_t nextCode = code | (static_cast<uint16_t>(br.peekBitMs().value()) << length );
                    
                   
                  
                        if (codes.contains(code) && l[codes.at(code)] == length)
                        {
                            //if(std::find(c.begin(), c.end(), codes.at(code)) == )
                            return codes.at(code);
                        }
                    
                }
                return NO_CODE;
            }


        private:
        };
    };
};

#endif