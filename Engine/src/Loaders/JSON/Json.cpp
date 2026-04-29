// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "Json.hpp"

namespace nb
{
    namespace Loaders
    {
        Ref<Resource::IResource> JsonFactory::create(
            const std::filesystem::path& path,
            nbstl::Span<std::string>     params
        ) const
        {
            return createRef<nb::Loaders::Json>(path);
        }
    };
};
