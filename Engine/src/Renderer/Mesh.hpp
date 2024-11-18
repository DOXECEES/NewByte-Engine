#ifndef SRC_RENDERER_MESH_HPP
#define SRC_RENDERER_MESH_HPP

#include "RendererStructures.hpp"
#include "OpenGL/VertexArray.hpp"

#include <vector>

namespace nb
{
    namespace Renderer
    {
        class Mesh
        {

        public:
            Mesh() = delete;
            Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices) noexcept;

            void draw() const noexcept;

        private:
            std::vector<Vertex> verticies;
            std::vector<uint32_t> indices;

            nb::OpenGl::VertexArray VAO;

            // texture
        };

    };
};

#endif