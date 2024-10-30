#include "Json.hpp"

namespace nb
{
    namespace Loaders
    {
        Ref<Resource::IResource> JsonFactory::create(const std::filesystem::path &path) const
        {
            return createRef<nb::Loaders::Json>(path);
        }
    };
};
