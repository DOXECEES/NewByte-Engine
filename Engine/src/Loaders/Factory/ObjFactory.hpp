#ifndef SRC_LOADERS_FACTORY_OBJFACTORY_HPP
#define SRC_LOADERS_FACTORY_OBJFACTORY_HPP

#include "../../Renderer/Mesh.hpp"
#include "IFactoryLoader.hpp"
#include "../ObjLoader.hpp"


#include "../../Core.hpp"

namespace nb
{
    namespace Loaders
    {
        namespace Factory
        {
            class ObjFactory : public IFactoryLoader
            {
                Ref<nb::Resource::IResource> create(const std::filesystem::path& path) const override;
            };
        };
    };
};

#endif

