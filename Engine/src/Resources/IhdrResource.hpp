#ifndef SRC_RESOURCES_IHDRRESOURCE_HPP
#define SRC_RESOURCES_IHDRRESOURCE_HPP

#include "Core.hpp"
#include "IResource.hpp"
#include "Renderer/Cubemap.hpp"
#include "Renderer/Texture.hpp"
#include <filesystem>

namespace nb::Resource
{

	class IhdrResource : public IResource
	{
	public:
            IhdrResource(
                const std::filesystem::path& path,
                Ref<nb::Renderer::Cubemap> cubemapTexture,
                Ref<nb::Renderer::Cubemap> irradianceTexture,
                Ref<nb::Renderer::Cubemap> prefilterTexture,
                Ref<nb::Renderer::Texture> brdfTexture
            )
		 : path(path)
		 , cubemap(cubemapTexture)
		 , irradiance(irradianceTexture)
         , prefilter(prefilterTexture)
         , brdf(brdfTexture)
         , IResource(path)
		{}
		virtual ~IhdrResource() = default;

		const Ref<nb::Renderer::Cubemap>& getCubemap() const
		{
			return cubemap;
		}

		const Ref<nb::Renderer::Cubemap>& getIrradianceCubemap() const
        {
            return irradiance;
        }

		const Ref<nb::Renderer::Cubemap>& getPrefilterCubemap() const
        {
            return prefilter;
        }

		const Ref<nb::Renderer::Texture>& getBrdfTexture() const
        {
            return brdf;
        }

	private:
		std::filesystem::path path;
		Ref<nb::Renderer::Cubemap> cubemap;
        Ref<nb::Renderer::Cubemap> irradiance;
        Ref<nb::Renderer::Cubemap> prefilter;
        Ref<nb::Renderer::Texture> brdf;


	};

};

#endif