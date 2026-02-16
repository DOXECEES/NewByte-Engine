#ifndef SRC_RENDERER_CONTEXTMESHCACHE_HPP
#define SRC_RENDERER_CONTEXTMESHCACHE_HPP

#include <Windows.h>

#include <unordered_map>
#include <Renderer/Mesh.hpp>

namespace nb::Renderer
{
	struct ContextMesh
	{
		GLuint vao;
		Ref<Mesh> source;
	};

	class ContextMeshCache
	{
	public:

		ContextMesh* get(HGLRC hglrc, const Mesh* mesh) noexcept;
		
		virtual ContextMesh* insertMesh(HGLRC hglrc, Ref<Mesh>) noexcept = 0;
	
	protected:

		std::unordered_map<
			HGLRC,
			std::unordered_map<const Mesh*, ContextMesh>
		> meshes;

	};
};

namespace nb::OpenGl
{
	class OpenglContextMeshCache : public Renderer::ContextMeshCache
	{
	private:
		Renderer::ContextMesh* insertMesh(HGLRC hglrc, Ref<Renderer::Mesh> mesh) noexcept override;
	};

};



#endif
