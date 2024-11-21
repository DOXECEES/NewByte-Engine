#include "ObjFactory.hpp"

namespace nb
{
    namespace Loaders
    {
        namespace Factory
        {
            Ref<nb::Resource::IResource> ObjFactory::create(const std::filesystem::path &path) const
            {
                if(path.extension() == ".obj")
                    return ObjLoader::loadMesh(path);
            }
        };
    };
};