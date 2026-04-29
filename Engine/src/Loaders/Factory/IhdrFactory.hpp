#ifndef SRC_LOADERS_FACTORY_IHDRFACTORY_HPP
#define SRC_LOADERS_FACTORY_IHDRFACTORY_HPP

#include <NonOwningPtr.hpp>

#include "IFactoryLoader.hpp"
#include "Loaders/IhdrLoader.hpp"
#include "Renderer/IRenderAPI.hpp"

namespace nb::Renderer
{
    class IRenderAPI;
}

namespace nb::Loaders::Factory
{
    class IhdrFactory : public IFactoryLoader
    {
    public:
        IhdrFactory(nbstl::NonOwningPtr<Renderer::IRenderAPI> renderApi) noexcept;

        Ref<nb::Resource::IResource> create(
            const std::filesystem::path& path,
            nbstl::Span<std::string> params = {}
        ) const override;
        std::type_index getResourceType() const noexcept override;

    private:
        mutable IhdrLoader loader;
        nbstl::NonOwningPtr<Renderer::IRenderAPI> renderApi;
    };
};

#endif