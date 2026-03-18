#ifndef SRC_RESOURCES_IHDRRESOURCE_HPP
#define SRC_RESOURCES_IHDRRESOURCE_HPP

#include "Core.hpp"
#include "IResource.hpp"
#include "Renderer/Cubemap.hpp"
#include <filesystem>

namespace nb::Resource
{

	class IhdrResource : public IResource
	{
	public:
		IhdrResource(const std::filesystem::path& path, Ref<nb::Renderer::Cubemap> cubemapTexture)
		 : path(path)
		 , cubemap(cubemapTexture)
		{}
		virtual ~IhdrResource() = default;

		const Ref<nb::Renderer::Cubemap>& getCubemap() const
		{
			return cubemap;
		}

	private:
		std::filesystem::path path;
		Ref<nb::Renderer::Cubemap> cubemap;

	};

};

#endif