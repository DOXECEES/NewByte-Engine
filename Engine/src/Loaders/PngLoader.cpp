
#include "PngLoader.hpp"

#include "../Utils/DeflateDecoder.hpp"

nb::Loaders::PngLoader::PngLoader(const std::string &filePath)
    : file(filePath, std::ios::binary)
{
    if (!validateHeader())
        isGood = false;
    // if (!validateIhdr()) isGood = false;
    while (true)
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
    using namespace nb::Utils;

    if (std::strncmp(chunk.name, "IHDR", 4) == 0)
    {
        Debug::debug("IHDR found");
    }
    else if (std::strncmp(chunk.name, "IDAT", 4) == 0)
    {
        Debug::debug("IDAT found");
        //nb::Utils::BitReader<char> br(chunk.data);

        DeflateDecoder dd(chunk.data);
#ifdef NB_DEBUG
        //assertEqual(17137U, chunk.data.size());
        assert(chunk.data.size() == 17137);
        
        auto zlibHeader = dd.readZlibHeader();
                
        DeflateDecoder::BlockHeader header = dd.readBlockHeader();
        switch (header.type)
        {
        case DeflateDecoder::BlockCompression::UNCOMPRESSED:
            Debug::debug("UNCOMPRESSED");
            break;
        
        case DeflateDecoder::BlockCompression::FIXED:
            Debug::debug("FIXED");
            break;
        
        case DeflateDecoder::BlockCompression::DYNAMIC:
            Debug::debug("DYNAMIC");
            dd.processDynamicHuffman();
            break;
        
        case DeflateDecoder::BlockCompression::RESERVED:
            Debug::debug("RESERVED");
            break;
        }

#endif
        //     Debug::debug(codes);
        //     // uint16_t bits = br.readRightToLeft(2);
        //     // uint16_t counter = 2;

        //     std::vector<uint32_t> litLenLengths(HLIT, 0);
        //     for (int counter = 0; counter < HLIT; ++counter)
        //     {
        //         uint16_t len = 0;
        //         code = 0; // Сбрасываем code перед каждой попыткой найти новый символ

        //         // Попытка найти код длины в диапазоне от 1 до 15 бит
        //         for (int length = 1; length <= 15; ++length)
        //         {
        //             code = (code << 1) | br.readRightToLeft(1); // Читаем по одному биту и сдвигаем

        //             if (codes.contains(code))
        //             {
        //                 len = codes[code]; // Найденный код длины
        //                 break;
        //             }
        //         }

        //         switch (len)
        //         {
        //         case 16: // Повторение последней длины
        //         {
        //             if (counter == 0)
        //                 throw std::runtime_error("Repeat length at start is invalid");

        //             int repeat = 3 + br.readRightToLeft(2); // Читаем 2 бита для повторения
        //             for (int j = 0; j < repeat && (counter + j) < HLIT; ++j)
        //             {
        //                 litLenLengths[counter + j] = litLenLengths[counter - 1];
        //             }
        //             counter += repeat - 1; // Корректируем счётчик
        //             break;
        //         }
        //         case 17: // Повторение нулевой длины
        //         {
        //             int repeat = 3 + br.readRightToLeft(3); // Читаем 3 бита для повторения
        //             counter += repeat - 1;                  // Корректируем счётчик
        //             break;
        //         }
        //         case 18: // Длинное повторение нулевой длины
        //         {
        //             int repeat = 11 + br.readRightToLeft(7); // Читаем 7 бит для длинного повторения
        //             counter += repeat - 1;                   // Корректируем счётчик
        //             break;
        //         }
        //         default: // Значение длины от 0 до 15
        //             litLenLengths[counter] = len;
        //             break;
        //         }
        //     }
        //     Debug::debug(litLenLengths);

        //     std::vector<uint32_t> distLengths(HDIST, 0);
        //     for (int counter = 0; counter < HDIST; ++counter)
        //     {
        //         uint16_t len = 0;
        //         code = 0;

        //         for (int length = 1; length <= 15; ++length)
        //         {
        //             code = (code << 1) | br.readRightToLeft(1);

        //             if (codes.contains(code))
        //             {
        //                 len = codes[code];
        //                 break;
        //             }
        //         }

        //         switch (len)
        //         {
        //         case 16:
        //         {
        //             if (counter == 0)
        //                 throw std::runtime_error("Repeat length at start is invalid");

        //             int repeat = 3 + br.readRightToLeft(2);
        //             for (int j = 0; j < repeat && (counter + j) < HDIST; ++j)
        //             {
        //                 distLengths[counter + j] = distLengths[counter - 1];
        //             }
        //             counter += repeat - 1;
        //             break;
        //         }
        //         case 17:
        //         {
        //             int repeat = 3 + br.readRightToLeft(3);
        //             counter += repeat - 1;
        //             break;
        //         }
        //         case 18:
        //         {
        //             int repeat = 11 + br.readRightToLeft(7);
        //             counter += repeat - 1;
        //             break;
        //         }
        //         default:
        //             distLengths[counter] = len;
        //             break;
        //         }

        //         if (counter >= HDIST)
        //         {
        //             throw std::runtime_error("Out of bounds access in distLengths");
        //         }
        //     }

        //     Debug::debug(distLengths);


    }
    // else if (std::strncmp(chunk.name, "IEND", 4) == 0)
    // {
    //     Debug::debug("IEND found");
    // }
}

void nb::Loaders::PngLoader::readChunk()
{
    std::array<uint8_t, CHUNK_SIZE_IN_BYTES> chunkSize;
    file.read(reinterpret_cast<char *>(&chunkSize), CHUNK_SIZE_IN_BYTES);

    uint32_t size = swapEndian(*reinterpret_cast<int *>(&chunkSize));

    ChunkData cd;

    file.read(cd.name, 4);

    cd.data.resize(size);
    file.read(cd.data.data(), size);
    file.read(reinterpret_cast<char *>(&cd.crc), 4);

    identifyChunk(cd);
}
