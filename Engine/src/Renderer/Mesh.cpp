#include "Mesh.hpp"

namespace nb
{
    namespace Renderer
    {

        Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, const std::vector<Renderer::Material>& mat) noexcept
            : verticies(std::move(vertices))
            , indices(std::move(indices))
            , materials(std::move(mat))
        {
            Math::Vector3<float> minPoint;
            Math::Vector3<float> maxPoint;

            for (const auto& vertex : vertices)
            {
                minPoint.x = std::min(minPoint.x, vertex.position.x);
                minPoint.y = std::min(minPoint.y, vertex.position.y);
                minPoint.z = std::min(minPoint.z, vertex.position.z);

                maxPoint.x = std::max(maxPoint.x, vertex.position.x);
                maxPoint.y = std::max(maxPoint.y, vertex.position.y);
                maxPoint.z = std::max(maxPoint.z, vertex.position.z);
            }

            aabb = Math::AABB3D(minPoint, maxPoint);

            VAO.linkData(vertices, indices);
        }

        void Mesh::draw(const GLenum mode) const noexcept
        {
            VAO.bind();
            VAO.draw(indices.size(), mode);
        }
        void Mesh::drawAabb() noexcept
        {
            std::vector<Renderer::Vertex> vertices = {
                {{aabb.minPoint.x, aabb.minPoint.y, aabb.minPoint.z},{}},
                {{aabb.minPoint.x, aabb.minPoint.y, aabb.maxPoint.z},{}},
                {{aabb.maxPoint.x, aabb.minPoint.y, aabb.maxPoint.z},{}},
                {{aabb.maxPoint.x, aabb.minPoint.y, aabb.minPoint.z},{}},
                {{aabb.minPoint.x, aabb.maxPoint.y, aabb.minPoint.z},{}},
                {{aabb.minPoint.x, aabb.maxPoint.y, aabb.maxPoint.z},{}},
                {{aabb.maxPoint.x, aabb.maxPoint.y, aabb.maxPoint.z},{}},
                {{aabb.maxPoint.x, aabb.maxPoint.y, aabb.minPoint.z},{}}
            };

            std::vector<unsigned int> edges = {
                0,4,
                1,5,
                2,6,
                3,7,

                0,1,
                1,2,
                2,3,
                3,0,

                4,5,
                5,6,
                6,7,
                7,4
            };

            Mesh m(vertices, edges);

            m.draw(GL_LINES);
        }
    };
};
