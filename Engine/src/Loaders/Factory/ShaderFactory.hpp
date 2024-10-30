#ifndef SRC_LOADERS_FACTORY_SHADERFACTORY_HPP
#define SRC_LOADERS_FACTORY_SHADERFACTORY_HPP

#include "IFactoryLoader.hpp"
#include "../../Resources/IResource.hpp"

#include "../../Core/EngineSettings.hpp"
#include "../../Core.hpp"

namespace nb
{
    namespace Loaders
    {
        namespace Factory
        {
            class ShaderFactory : public Factory::IFactoryLoader
            {
                Ref<nb::Resource::IResource> create(const std::filesystem::path& path) const override;
            };
        };

    };
};

#endif
