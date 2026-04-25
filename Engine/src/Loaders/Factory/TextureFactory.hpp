#ifndef SRC_LOADERS_FACTORY_TEXTUREFACTORY_HPP
#define SRC_LOADERS_FACTORY_TEXTUREFACTORY_HPP

#include "IFactoryLoader.hpp"
#include "Loaders/TextureLoader.hpp"
#include "Resources/IResource.hpp"
#include "Resources/TextureAsset.hpp"



namespace nb::Loaders::Factory
{

    class TextureFactory : public Factory::IFactoryLoader
    {
    public:
        Ref<nb::Resource::IResource> create(
            const std::filesystem::path& path,
            nbstl::Span<std::string>     params = {}
        ) const override;
        std::type_index getResourceType() const noexcept override
        {
            return std::type_index(typeid(nb::Resource::TextureAsset));
        }
    private:
        mutable TextureLoader textureLoader;
    };
};

#endif
