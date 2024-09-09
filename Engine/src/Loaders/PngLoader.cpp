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
        Debug::debug(br.readRightToLeft(4));
        Debug::debug(br.readRightToLeft(4));
        Debug::debug(br.readRightToLeft(5));
        Debug::debug(br.readRightToLeft(1));
        Debug::debug(br.readRightToLeft(2));

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
