#include "TextureLoader.hpp"
#include "Resources/TextureAsset.hpp"

#include "JSON/Json.hpp"

namespace nb::Loaders
{
    Ref<Resource::TextureAsset> TextureLoader::loadAsset(const std::filesystem::path& path) noexcept
    {
        std::filesystem::path pngPath = path;
        if (std::filesystem::exists(pngPath))
        {
            pngPath.replace_extension(".jpeg");

        }
        else
        {
            pngPath.replace_extension(".png");
        }

        //pngPath.replace_extension(".png");
        std::filesystem::path metaPath = pngPath.string() + ".nbmeta";

        Resource::TextureSettings settings;
        if (std::filesystem::exists(metaPath)) 
        {
            settings = parseMetaFile(metaPath);
        } 
        else 
        {
            settings = Resource::TextureSettings(); 
        }

        auto textureAsset = createRef<Resource::TextureAsset>(pngPath, settings);

        return textureAsset;
    }

    Resource::TextureSettings TextureLoader::parseMetaFile(const std::filesystem::path& path) noexcept
    {
        Json json(path);
        Resource::TextureSettings settings;

        settings.exposure = json["exposure"].get<float>();
        settings.gamma = json["gamma"].get<float>();
        
        return settings;
    }


};