// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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
                {
                    return ObjLoader::loadMesh(path);
                }

                return nullptr;
            }
        };
    };
};