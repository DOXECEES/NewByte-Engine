#ifndef SRC_LOADERS_BITMAPLOADER_HPP
#define SRC_LOADERS_BITMAPLOADER_HPP

#include <fstream>
#include <string_view>

namespace nb
{
    namespace Loaders
    {

        template <typename T>
        void readToStruct(std::ifstream &stream, T &structure)
        {
            stream.read(reinterpret_cast<char *>(std::addressof(structure)), sizeof(T));
        }

        class BitmapLoader
        {
        public:
            enum class BitCount : uint16_t
            {
                MONOCHROME_PALETTE = 1,
                BIT_4_PALETTIZED = 4,
                BIT_8_PALETTIZED = 8,
                BIT_16_RGB = 16,
                BIT_24_RGB = 24,
            };

            enum class Compression : uint8_t
            {
                NB_BI_RGB = 0,
                NB_BI_RLE8 = 1,
                NB_BI_RLE4 = 2,
            };

            enum class ColorSpace
            {
                CALIBRATED_RGB = 0x0,
                SRGB = 0x73524742,
                WINDOWS_COLOR_SPACE = 0x57696E20,
                LINKED = 0x4C494E4B,
                EMBEDDED = 0x4D424544,
            };

#pragma pack(push, 1)

            struct BMPHeader
            {
                uint16_t header = 0;
                uint32_t fileSize = 0;
                uint32_t reserved = 0;
                uint32_t dataOffset = 0;
            };

            struct BMPInfoHeader
            {
                uint32_t size = 0;
                uint32_t width = 0;
                uint32_t height = 0;
                uint16_t planes = 1;
                uint16_t bitsPerPixel = 0;
                BitCount bitCount = BitCount::MONOCHROME_PALETTE;
                Compression compression = Compression::NB_BI_RGB;
                uint32_t imageSize = 0;
                uint32_t pixelsX = 0;
                uint32_t pixelsY = 0;
                uint32_t usedColors = 0;
                uint32_t importantColors = 0;
            };

            struct BMPInfoHeaderV4
            {
                uint32_t size = 0;
                uint32_t width = 0;
                uint32_t height = 0;
                uint16_t planes = 1;
                uint16_t bitsPerPixel = 0;
                BitCount bitCount = BitCount::MONOCHROME_PALETTE;
                Compression compression = Compression::NB_BI_RGB;
                uint32_t imageSize = 0;
                uint32_t pixelsX = 0;
                uint32_t pixelsY = 0;
                uint32_t usedColors = 0;
                uint32_t importantColors = 0;
                uint32_t redMask = 0;
                uint32_t greenMask = 0;
                uint32_t blueMask = 0;
                uint32_t alphaMask = 0;
                ColorSpace colorSpace = ColorSpace::WINDOWS_COLOR_SPACE;

                uint32_t gammaRed = 0;
                uint32_t gammaGreen = 0;
                uint32_t gammaBlue = 0;
            };

            struct ColorTable
            {
                uint8_t red = 1;
                uint8_t green = 1;
                uint8_t blue = 1;
                uint8_t reserved = 0;
            };

#pragma pack(pop)

            BitmapLoader() = delete;
            explicit BitmapLoader(std::string_view path);

        private:
            std::ifstream file;
        };

    };
};

#endif