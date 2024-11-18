#include "Mesh.hpp"

namespace nb
{
    namespace Renderer
    {

        Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices) noexcept
            : verticies(std::move(vertices))
            , indices(std::move(indices))
        {
            VAO.linkData(vertices, indices);
        }

        void Mesh::draw() const noexcept
        {
            VAO.bind();
            VAO.draw(indices.size());
        }
    };
};
