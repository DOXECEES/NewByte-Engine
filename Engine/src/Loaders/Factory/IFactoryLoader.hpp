#ifndef SRC_LOADERS_FACTORY_IFACTORYLOADER_HPP
#define SRC_LOADERS_FACTORY_IFACTORYLOADER_HPP

#include <filesystem>

#include "../../Core.hpp"

#include "../../Resources/IResource.hpp"

namespace nb
{
    namespace Loaders
    {
        namespace Factory
        {
            class IFactoryLoader
            {
            public:
                virtual Ref<nb::Resource::IResource> create(const std::filesystem::path& path) const = 0;
            };
        };
    };
};

#endif
