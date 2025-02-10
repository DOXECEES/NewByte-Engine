#include "Mesh.hpp"
#include "SceneGraph.hpp"
namespace nb
{
    namespace Renderer
    {

        SubMesh::SubMesh(const std::vector<uint32_t> &indices, const Renderer::Material& mat) noexcept
            //: verticies(std::move(vertices))
            : indices(std::move(indices))
            , material(mat)
        {
            //Math::Vector3<float> minPoint;
            //Math::Vector3<float> maxPoint;

            // for (const auto& vertex : *vertices)
            // {
            //     minPoint.x = std::min(minPoint.x, vertex.position.x);
            //     minPoint.y = std::min(minPoint.y, vertex.position.y);
            //     minPoint.z = std::min(minPoint.z, vertex.position.z);

            //     maxPoint.x = std::max(maxPoint.x, vertex.position.x);
            //     maxPoint.y = std::max(maxPoint.y, vertex.position.y);
            //     maxPoint.z = std::max(maxPoint.z, vertex.position.z);
            // }

            // aabb = Math::AABB3D(minPoint, maxPoint);

            //VAO.linkData(*vertices, indices);

            shader = std::make_shared<ShaderUniforms>();

            shader->vec3Uniforms["Ka"] = mat.ambient;
            shader->vec3Uniforms["Kd"] = mat.diffuse;
            shader->vec3Uniforms["Ks"] = mat.specular;
            shader->floatUniforms["shine"] = mat.shininess;
            shader->floatUniforms["d"] = mat.dissolve; // add other
        }

        void SubMesh::attachShader(const Ref<Shader> &shader) noexcept
        {
            this->shader->shader = shader;
        }

        // void SubMesh::draw(const GLenum mode) const noexcept
        // {
        //     //VAO.bind();
        //     //VAO.draw(indices.size(), mode);
        // }
        // void SubMesh::drawAabb() noexcept
        // {
        //     std::vector<Renderer::Vertex> vertices = {
        //         {{aabb.minPoint.x, aabb.minPoint.y, aabb.minPoint.z},{}},
        //         {{aabb.minPoint.x, aabb.minPoint.y, aabb.maxPoint.z},{}},
        //         {{aabb.maxPoint.x, aabb.minPoint.y, aabb.maxPoint.z},{}},
        //         {{aabb.maxPoint.x, aabb.minPoint.y, aabb.minPoint.z},{}},
        //         {{aabb.minPoint.x, aabb.maxPoint.y, aabb.minPoint.z},{}},
        //         {{aabb.minPoint.x, aabb.maxPoint.y, aabb.maxPoint.z},{}},
        //         {{aabb.maxPoint.x, aabb.maxPoint.y, aabb.maxPoint.z},{}},
        //         {{aabb.maxPoint.x, aabb.maxPoint.y, aabb.minPoint.z},{}}
        //     };

        //     std::vector<unsigned int> edges = {
        //         0,4,
        //         1,5,
        //         2,6,
        //         3,7,

        //         0,1,
        //         1,2,
        //         2,3,
        //         3,0,

        //         4,5,
        //         5,6,
        //         6,7,
        //         7,4
        //     };

        //     //SubMesh m(edges);

        //     //m.draw(GL_LINES);
        // }

        /// MESH

        // Mesh::Mesh(const std::vector<SubMesh> &meshes)
        //     : meshes(std::move(meshes))
        // {

        // }

        Mesh::Mesh(const std::vector<Vertex> &vert, const std::vector<uint32_t> &ind)
            : verticies(vert)
        {
            meshes.push_back(std::make_unique<SubMesh>(ind));
            VAO.linkData(verticies, ind);
        }

        std::vector<Renderer::Material> Mesh::getMaterials() const noexcept
        {
            std::vector<Renderer::Material> mats;
            for(const auto& i : meshes)
            {
                mats.push_back(i->getMaterial());
            }

            return std::move(mats);
        }

        std::vector<uint32_t> Mesh::uniteIndicies() noexcept
        {
            size_t lenght = 0;
            for(const auto& i : meshes)
                lenght += i->indices.size();

            indiciesCount = lenght;

            std::vector<uint32_t> unitedVector(0, lenght);
            for(const auto& i : meshes)
            {
                unitedVector.insert(
                    unitedVector.end(),
                    std::make_move_iterator(i->indices.begin()),
                    std::make_move_iterator(i->indices.end())
                );
            }       

            return unitedVector;
        }

        void Mesh::applyMaterial(const SubMesh& mesh) const noexcept
        {
            mesh.shader->applyUniforms();
            mesh.shader->shader->use();
        }

        void Mesh::draw(GLenum mode, const Ref<Shader>& shader) const noexcept
        {
            size_t indiciesOffset = 0;
            size_t matIndex = 0;
            for(const auto& i : meshes)
            {

                i->attachShader(shader);
                applyMaterial(*i);

                VAO.bind();
                VAO.draw(i->indices.size(), mode, indiciesOffset);
                indiciesOffset += i->indices.size();

                // // activate material
                // if(i->material == Renderer::Material())
                // {   
                //     n->mt[0]->applyMaterial();
                //     n->mt[0]->shader->use();
                // }
                // else
                // {
                //     n->mt[1]->applyMaterial();
                //     n->mt[1]->shader->use();
                // }
            }
        }
    };
};
