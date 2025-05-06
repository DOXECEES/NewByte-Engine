#include "DeflateDecoder.hpp"
#include "../Shared/HuffmanTree.hpp"

#include "../Debug.hpp"

nb::Utils::DeflateDecoder::DeflateDecoder(const std::vector<char> &vec)
    :br(vec)
{
}

nb::Utils::DeflateDecoder::ZlibHeader nb::Utils::DeflateDecoder::readZlibHeader()
{
    ZlibHeader zh;
    zh.cm = br.readRightToLeft(4);
    zh.cinfo = br.readRightToLeft(4);
    zh.fcheak = br.readRightToLeft(5);
    zh.fdict = br.readRightToLeft(1);
    zh.flevel = br.readRightToLeft(2);
    return zh;
}

void nb::Utils::DeflateDecoder::processDynamicHuffman()
{
    uint16_t HLIT = br.readRightToLeft(5) + 257;
    uint16_t HDIST = br.readRightToLeft(5) + 1;
    uint16_t HLEN = br.readRightToLeft(4) + 4;

    std::vector<std::pair<uint16_t, uint16_t>> idToLength(HLEN);

    for (size_t i = 0; i < HLEN; i++)
    {
        idToLength[i] = std::make_pair(COMMAND_ALPHABET_SEQUENCE[i], br.readRightToLeft(3));
    }

    nb::Shared::HuffmanTree hTree;
    auto codesToId = hTree.buildCanonicalCodesByLength(idToLength);

    Debug::debug("CODES:");
    Debug::debug(codesToId);
    // Debug::debug(br.getBytePointer());
    // Debug::debug(int(br.getMsBitPointer()));
    // Debug::debug(int(br.getBitMs().value()));
    // Debug::debug(br.readLeftToRight(3));

    // Debug::debug(br.getBytePointer());
    // Debug::debug(int(br.getMsBitPointer()));

    // Debug::debug(br.readLeftToRight(8));

    std::vector<uint16_t> litLenAlphabet(HLIT, 0);
    for (size_t i = 0; i < HLIT; i++)
    {
        auto code = hTree.decodeCanonical(br, codesToId, idToLength);

        switch(code)
        {
            case 16:
            {
                uint8_t repeat = 3 + br.readRightToLeft(2);
                std::fill(litLenAlphabet.begin() + i, litLenAlphabet.begin() + repeat, litLenAlphabet[i-1]);
                i += repeat;
                break;
            }
            case 17: 
            {
                uint8_t repeat = 3 + br.readRightToLeft(3);
                i += repeat;
                break;
            }
            case 18: 
            {
                uint16_t repeat = 3 + br.readRightToLeft(7);
                i += repeat;
                break;
            }
            default:
            {
                litLenAlphabet[i++] = code;
                break;
            }
        }

    }
    Debug::debug(litLenAlphabet);

    //
    // TODO: класс для дерева Хаффмана с декодированием,
    // созданием канонических кодов, построением hash-map и дерева
    // 
}

nb::Utils::DeflateDecoder::BlockHeader nb::Utils::DeflateDecoder::readBlockHeader()
{
    BlockHeader header;
    header.isFinal = br.readRightToLeft(1);
    header.type = static_cast<BlockCompression>(br.readRightToLeft(2));
    return header;
}
