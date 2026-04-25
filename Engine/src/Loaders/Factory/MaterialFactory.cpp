#include "MaterialFactory.hpp"


namespace nb::Loaders::Factory
{
    Ref<nb::Resource::IResource> MaterialFactory::create(
        const std::filesystem::path& path,
        nbstl::Span<std::string>     params
    ) const
    {
        if (path.extension() == ".material")
        {
            return materialLoader.loadAsset(path);
        }
        return nullptr;
    }

};