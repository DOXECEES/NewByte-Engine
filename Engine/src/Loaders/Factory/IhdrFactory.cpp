
#include "IhdrFactory.hpp"
#include "Loaders/IhdrLoader.hpp"
#include "Renderer/IFrameBuffer.hpp"
#include "Resources/IhdrResource.hpp"

#include <Renderer/IRenderAPI.hpp>

namespace nb::Loaders::Factory
{
    IhdrFactory::IhdrFactory(nbstl::NonOwningPtr<Renderer::IRenderAPI> renderApi) noexcept
        : renderApi(renderApi)
    {
    }

    Ref<nb::Resource::IResource> IhdrFactory::create(const std::filesystem::path& path) const
    {
        if (path.extension() != ".hdr")
        {
            nb::Error::ErrorManager::instance()
                .report(nb::Error::Type::WARNING, "File extention not supported")
                .with("path", path.string());
                return nullptr;
        }

        IhdrLoader::ImageData data = loader.loadFromFile(path);
        Renderer::TextureDescriptor descriptor
        {
            .width = data.width,
            .height = data.height,
            .format = Renderer::TextureFormat::RGB16F,
            .data = data.data
        };

        auto texture2d = renderApi->createTexture2d(descriptor);

        auto cubemap = renderApi->bakeTextureIntoCubeMap(texture2d);

        auto irradiance = renderApi->bakeIrradiance(cubemap);

        auto prefilter = renderApi->bakePrefilter(cubemap);

        auto brdfTexture = renderApi->bakeBRDF();


        return createRef<nb::Resource::IhdrResource>(
            path, cubemap, irradiance, prefilter, brdfTexture
        );
    }

    std::type_index IhdrFactory::getResourceType() const noexcept
    {
        return std::type_index(typeid(Resource::IhdrResource));
    }
};

