// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "ObjFactory.hpp"
#include "Loaders/AsimpLoader.hpp"
namespace nb
{
    namespace Loaders
    {
        namespace Factory
        {
            Ref<nb::Resource::IResource> ObjFactory::create(
                const std::filesystem::path& path,
                nbstl::Span<std::string>     params
            ) const
            {
                if(path.extension() == ".obj")
                {
                    return ObjLoader::loadMesh(path);
                }
                else if (path.extension() == ".fbx")
                {
                    return nb::Renderer::AssimpLoader::load(path);
                }

                return nullptr;
            }
        };
    };
};