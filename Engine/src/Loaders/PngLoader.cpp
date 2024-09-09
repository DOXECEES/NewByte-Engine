#include "PngLoader.hpp"

nb::Loaders::PngLoader::PngLoader(const std::string &filePath)
    : file(filePath, std::ios::binary)
{
    if (!validateHeader()) isGood = false;
    //if (!validateIhdr()) isGood = false;
    while(true)
        readChunk();
}

bool nb::Loaders::PngLoader::validateHeader()
{
    std::array<uint8_t, PNG_HEADER_SIZE> arr;
    file.read(reinterpret_cast<char *>(&arr), PNG_HEADER_SIZE);

    return (arr == PNG_HEADER);
}

bool nb::Loaders::PngLoader::validateIhdr()
{
    return true;
}

nb::Loaders::PngLoader::ZlibHeader nb::Loaders::PngLoader::readZlibHeader(BitReader &br)
{
    ZlibHeader zh;
    zh.cm = br.readRightToLeft(4);
    zh.cinfo = br.readRightToLeft(4);
    zh.fcheak = br.readRightToLeft(5);
    zh.fdict = br.readRightToLeft(1);
    zh.flevel = br.readRightToLeft(2);
    return zh;
}

void nb::Loaders::PngLoader::identifyChunk(const ChunkData &chunk)
{
    if(std::strncmp(chunk.name, "IHDR",4) == 0)
    {
        Debug::debug("IHDR found");
    }
    else if(std::strncmp(chunk.name, "IDAT", 4) == 0)
    {
        Debug::debug("IDAT found");
        BitReader br(chunk.data);

        auto zlibHeader = readZlibHeader(br);

        uint16_t HLIT = br.readRightToLeft(5) + 257;
        uint16_t HDIST = br.readRightToLeft(5) + 1;
        uint16_t HLEN = br.readRightToLeft(4) + 4;

        std::vector<std::pair<uint16_t, uint16_t>> commandAlphabet(HLEN);

        for (size_t i = 0; i < MAX_COMMAND_ALPHABET_SIZE; i++)
        {
            commandAlphabet[i] = std::make_pair(COMMAND_ALPHABET_SEQUENCE[i], br.readRightToLeft(3));
        }

        commandAlphabet.erase(std::remove_if(
            commandAlphabet.begin(), commandAlphabet.end(),
            [](const std::pair<uint16_t, uint16_t>& x) { 
                return x.second == 0;
            }), commandAlphabet.end());

        std::sort(commandAlphabet.begin(), commandAlphabet.end(), [](const std::pair<uint16_t, uint16_t> &a, const std::pair<uint16_t, uint16_t> &b) -> bool
                  {
                      return a.second < b.second;
                  });

        // generate codes

        std::vector<uint16_t> codes;
        uint16_t code = 0;
        uint16_t prevLength = 0;

        for(auto& i: commandAlphabet)
        {
            if(i.second > prevLength)
            {
                code = code << (i.second - prevLength);
                prevLength = i.second;
            }
            codes.push_back(code);
            code += 1;
        }

        


        Debug::debug(codes);
    }
    else if (std::strncmp(chunk.name, "IEND", 4) == 0)
    {
        Debug::debug("IEND found");
    }
    
}

void nb::Loaders::PngLoader::readChunk()
{
    std::array<uint8_t, CHUNK_SIZE_IN_BYTES> chunkSize;
    file.read(reinterpret_cast<char *>(&chunkSize), CHUNK_SIZE_IN_BYTES);

    uint32_t size = swapEndian(*reinterpret_cast<int*>(&chunkSize));

    ChunkData cd;

    file.read(cd.name, 4);

    cd.data.resize(size);
    file.read(cd.data.data(), size);
    file.read(reinterpret_cast<char*>(&cd.crc), 4);

    identifyChunk(cd);
}



nb::Loaders::BitReader::BitReader(const std::vector<char> &vec)
    :data(vec.data())
    ,dataSize(vec.size())
{
}

bool nb::Loaders::BitReader::peekBit()
{
    currentPointerPos++;
    return (currentPointerPos < dataSize) ? (data[(currentPointerPos-1) / 8] & (1 << ((currentPointerPos - 1) % 8))) : false;
}

uint32_t nb::Loaders::BitReader::readLeftToRight(const uint8_t count)
{
    uint32_t res = 0;

    for (size_t i = 0; i < count; i++)
    {
        res |= (peekBit() << (count - i - 1));
    }

    return res;
}

uint32_t nb::Loaders::BitReader::readRightToLeft(const uint8_t count)
{
    uint32_t res = 0;

    for (size_t i = 0; i < count; i++)
    {
        res |= (peekBit() << i);
    }

    return res;
}
