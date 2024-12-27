#ifndef SRC_RENDERER_MESH_HPP
#define SRC_RENDERER_MESH_HPP

#include "RendererStructures.hpp"
#include "OpenGL/VertexArray.hpp"

#include "../Resources/IResource.hpp"

#include "../Math/AABB3D.hpp"

#include <vector>

namespace nb
{
    namespace Renderer
    {
        class Mesh : public Resource::IResource
        {

        public:
            Mesh() = delete;
            Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, const std::vector<Renderer::Material>& mat = {}) noexcept;

            inline const Math::AABB3D &getAABB() const noexcept { return aabb; };
            inline const std::vector<Renderer::Material> &getMaterials() const noexcept { return materials; };
            void draw(const GLenum mode) const noexcept;
            void drawAabb() noexcept;

        private:
            std::vector<Vertex> verticies;
            std::vector<uint32_t> indices;
            std::vector<Renderer::Material> materials;

            Math::AABB3D aabb;

            nb::OpenGl::VertexArray VAO;

        };

    };
};

#endif