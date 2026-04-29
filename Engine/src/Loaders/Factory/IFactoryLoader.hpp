#ifndef SRC_LOADERS_FACTORY_IFACTORYLOADER_HPP
#define SRC_LOADERS_FACTORY_IFACTORYLOADER_HPP

#include <filesystem>
#include <typeindex>
#include <Span.hpp>
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
                virtual Ref<nb::Resource::IResource> create(
                    const std::filesystem::path& path,
                    nbstl::Span<std::string>     params = {}
                ) const                                                  = 0;
                virtual std::type_index getResourceType() const noexcept                                = 0;
            };
        };
    };
};

#endif
