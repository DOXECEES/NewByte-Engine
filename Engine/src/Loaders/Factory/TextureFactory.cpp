#include "TextureFactory.hpp"

#include "Loaders/TextureLoader.hpp"

namespace nb::Loaders::Factory
{
    Ref<nb::Resource::IResource> TextureFactory::create(
        const std::filesystem::path& path,
        nbstl::Span<std::string>     params
    ) const
    {
        if(path.extension() == ".texture")
        {
            return textureLoader.loadAsset(path);
        }
        return nullptr;
    }

};