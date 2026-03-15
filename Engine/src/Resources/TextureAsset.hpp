#ifndef SRC_RESOURCES_TEXTUREASSET_HPP
#define SRC_RESOURCES_TEXTUREASSET_HPP

#include <NbCore.hpp>

#include <filesystem>
#include <memory>

#include "Renderer/Texture.hpp"

#include <NonOwningPtr.hpp>

#include "Renderer/OpenGL/OpenGlTexture.hpp"
#include "IResource.hpp"

namespace nb::Resource
{

    struct TextureSettings 
    {
        float exposure = 1.0f;
        float gamma = 2.2f;
    };

    class TextureAsset : public IResource
    {
    public:

        TextureAsset(
            const std::filesystem::path& path,
            const TextureSettings& settings
        ) 
            : path(path)
            , settings(settings)
        {}

        nbstl::NonOwningPtr<nb::Renderer::Texture> getInternalTexture()
        {
            if (!texture)
            {
                texture = std::make_unique<nb::OpenGl::OpenGlTexture>(path);
            }
            return texture.get();
        }

        const TextureSettings& getSettings() const { return settings; }

    private:
        std::filesystem::path path;     
        TextureSettings settings;       
        
        std::unique_ptr<nb::Renderer::Texture> texture; 
    };

};


#endif