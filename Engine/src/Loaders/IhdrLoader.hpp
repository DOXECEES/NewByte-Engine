#ifndef SRC_LOADERS_IHDRLOADER_HPP
#define SRC_LOADERS_IHDRLOADER_HPP

#include <filesystem>

namespace nb::Loaders
{
    class IhdrLoader
    {
    public:
        struct ImageData
        {
            int width;
            int height;
            int components;
            float* data;

            ~ImageData() noexcept;
        };

        IhdrLoader() noexcept = default;
        ~IhdrLoader() noexcept = default;

        ImageData loadFromFile(const std::filesystem::path& path) noexcept;

    private:
    };
} ;

#endif