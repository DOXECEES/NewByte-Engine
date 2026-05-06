#include "TextureLoader.hpp"
#include "Resources/TextureAsset.hpp"

#include "JSON/Json.hpp"

namespace nb::Loaders
{
    Ref<Resource::TextureAsset> TextureLoader::loadAsset(const std::filesystem::path& path) noexcept
    {
        std::filesystem::path pngPath = path;
        Resource::TextureSettings settings = {};


        if (std::filesystem::exists(pngPath))
        {
            Json json(path);
            pngPath = json["source"].get<std::string>();

            if (json.contains("wrap_u"))
            {
                settings.parameters.wrapU = static_cast<Renderer::Wrapping>(json["wrap_u"].get<int>());
            }

            if (json.contains("wrap_v"))
            {
                settings.parameters.wrapV = static_cast<Renderer::Wrapping>(json["wrap_v"].get<int>());
            }

            if (json.contains("min_filter"))
            {
                settings.parameters.minFilter = static_cast<Renderer::Filtering>(json["min_filter"].get<int>());
            }

            if (json.contains("mag_filter"))
            {
                settings.parameters.magFilter = static_cast<Renderer::Filtering>(json["mag_filter"].get<int>());
            }

            if (json.contains("use_mipmap"))
            {
                settings.parameters.generateMipmaps = json["use_mipmap"].get<bool>();
            }

            if (json.contains("should_flip"))
            {
                settings.parameters.shouldFlip = json["should_flip"].get<bool>();
            }
            //pngPath.replace_extension(".jpeg");
        }
        else
        {
            pngPath.replace_extension(".png");
        }

        //pngPath.replace_extension(".png");
        std::filesystem::path metaPath = pngPath.string() + ".nbmeta";

        //Resource::TextureSettings settings;
        if (std::filesystem::exists(metaPath)) 
        {
            settings = parseMetaFile(metaPath);
        } 
        else 
        {
            //settings = Resource::TextureSettings(); 
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