#ifndef SRC_LOADERS_FACTORY_SHADERFACTORY_HPP
#define SRC_LOADERS_FACTORY_SHADERFACTORY_HPP

#include "IFactoryLoader.hpp"
#include "../../Resources/IResource.hpp"

#include "../../Renderer/OpenGL/OpenGlShader.hpp"
#include "../../Manager/ResourceManager.hpp"

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
                std::type_index getResourceType() const noexcept override { return std::type_index(typeid(nb::Renderer::Shader)); }
            };
        };

    };
};

#endif
