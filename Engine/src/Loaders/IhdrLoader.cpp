#include "IhdrLoader.hpp"
#define STB_IMAGE_IMPLEMENTATION 
#include "../../dependencies/stb/stb_image.h"

namespace nb::Loaders
{
    IhdrLoader::ImageData IhdrLoader::loadFromFile(const std::filesystem::path& path) noexcept
    {
        ImageData data;

        data.data = stbi_loadf(
            path.string().c_str(),
            &data.width,
            &data.height,
            &data.components,
            0
        );

        return data;
    }

    IhdrLoader::ImageData::~ImageData() noexcept
    {
        if (data)
        {
            stbi_image_free(data);
        }
    }
};