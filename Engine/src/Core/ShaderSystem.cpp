// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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