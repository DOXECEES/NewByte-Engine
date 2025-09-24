#include "ShaderSystem.hpp"
#include "Manager/ResourceManager.hpp"

namespace nb
{
	void ShaderSystem::reloadAll() const noexcept
	{
		using namespace nb::ResMan;
		auto rm = ResourceManager::getInstance();
		const ResourceManager::ResourcePool& res = rm->getAllResources<nb::Renderer::Shader>();

		for (auto& i : res)
		{
			Resource::resourceTo<nb::Renderer::Shader>(i.second.get())->recompile();
		}
	}

};