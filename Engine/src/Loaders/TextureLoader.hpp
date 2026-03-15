#ifndef SRC_LOADERS_TEXTURELOADER_HPP
#define SRC_LOADERS_TEXTURELOADER_HPP

#include "Resources/TextureAsset.hpp"

namespace nb::Loaders
{
    class TextureLoader
    {
    public:
        TextureLoader() = default;
        ~TextureLoader() = default;

        Ref<Resource::TextureAsset> loadAsset(const std::filesystem::path& path) noexcept;

    private:

        Resource::TextureSettings parseMetaFile(const std::filesystem::path& path) noexcept;
        
    };
};

#endif