#ifndef SRC_LOADERS_FACTORY_MATERIALFACTORY_HPP
#define SRC_LOADERS_FACTORY_MATERIALFACTORY_HPP

#include "IFactoryLoader.hpp"
#include "Loaders/MaterialLoader.hpp"
#include "Resources/MaterialAsset.hpp"
#include <filesystem>


namespace nb::Loaders::Factory
{
    class MaterialFactory : public Factory::IFactoryLoader
    {
    public:
        Ref<nb::Resource::IResource> create(
            const std::filesystem::path& path,
            nbstl::Span<std::string>     params = {}
        ) const override;
        std::type_index getResourceType() const noexcept override
        {
            return std::type_index(typeid(nb::Resource::MaterialAsset));
        }

    private:
        mutable MaterialLoader materialLoader;
    };
}; 

#endif