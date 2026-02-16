// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "Mesh.hpp"
#include "SceneGraph.hpp"
namespace nb
{
    namespace Renderer
    {

        SubMesh::SubMesh(const std::vector<uint32_t> &indices, const Renderer::Material &mat) noexcept
            //: verticies(std::move(vertices))
            : indices(std::move(indices)), material(mat)
        {
            // Math::Vector3<float> minPoint;
            // Math::Vector3<float> maxPoint;

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

            // VAO.linkData(*vertices, indices);

            shader = std::make_shared<ShaderUniforms>();

            shader->vec3Uniforms["Ka"] = mat.ambient;
            shader->vec3Uniforms["Kd"] = mat.diffuse;
            shader->vec3Uniforms["Ks"] = mat.specular;
            shader->floatUniforms["shine"] = mat.shininess;
            // shader->floatUniforms["d"] = mat.dissolve; // add other
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
            calculateTagnentArray();
            
            VAO.linkData(verticies, ind);
            recalculateAabb3dForce();
        }

        Mesh::Mesh(const Mesh& other) noexcept
            : Resource::IResource(other) 
            , aabb(other.aabb)
            , ind(other.ind)
            , verticies(other.verticies)
            , indiciesCount(other.indiciesCount)
            , uniforms(other.uniforms)
            , VAO(other.VAO)
        {
           
            meshes.reserve(other.meshes.size());
            for (const auto& sub : other.meshes)
            {
                auto newSub = std::make_unique<SubMesh>(sub->indices, sub->material);
                newSub->shader = sub->shader; 

                this->meshes.push_back(std::move(newSub));
            }

        }


        std::vector<Renderer::Material> Mesh::getMaterials() const noexcept
        {
            std::vector<Renderer::Material> mats;
            for (const auto &i : meshes)
            {
                mats.push_back(i->getMaterial());
            }

            return std::move(mats);
        }

        const Math::AABB3D &Mesh::getAabb3d() const noexcept
        {
            return aabb;
        }

        const Math::AABB3D &Mesh::recalculateAabb3dForce() noexcept
        {

            Math::Vector3<float> minPoint;
            Math::Vector3<float> maxPoint;

            for (const auto &vertex : verticies)
            {
                minPoint.x = (std::min)(minPoint.x, vertex.position.x);
                minPoint.y = (std::min)(minPoint.y, vertex.position.y);
                minPoint.z = (std::min)(minPoint.z, vertex.position.z);

                maxPoint.x = (std::max)(maxPoint.x, vertex.position.x);
                maxPoint.y = (std::max)(maxPoint.y, vertex.position.y);
                maxPoint.z = (std::max)(maxPoint.z, vertex.position.z);
            }

            aabb = Math::AABB3D(minPoint, maxPoint);
            return aabb;
        }

        GLuint Mesh::getVboId() const noexcept
        {
            return VAO.getVbo().getId();
        }

        GLuint Mesh::getEboId() const noexcept
        {
            return VAO.getEbo().getId();
        }

        std::vector<uint32_t> Mesh::uniteIndicies() noexcept
        {
            size_t lenght = 0;
            for (const auto &i : meshes)
                lenght += i->indices.size();

            indiciesCount = lenght;

            std::vector<uint32_t> unitedVector(0, lenght);
            for (const auto &i : meshes)
            {
                unitedVector.insert(
                    unitedVector.end(),
                    std::make_move_iterator(i->indices.begin()),
                    std::make_move_iterator(i->indices.end()));
            }

            return unitedVector;
        }

        void Mesh::applyMaterial(const SubMesh &mesh) const noexcept
        {
            mesh.shader->applyUniforms();
            mesh.shader->applyUniforms(uniforms);
            mesh.shader->shader->use();
        }

        void Mesh::draw(GLenum mode, const Ref<Shader> &shader) const noexcept
        {
            VAO.bind();

            size_t byteOffset = 0;

            for (const auto& sub : meshes)
            {
                sub->attachShader(shader);
                applyMaterial(*sub);

                VAO.draw(static_cast<uint32_t>(sub->indices.size()), mode, byteOffset);

                byteOffset += sub->indices.size() * sizeof(uint32_t);
            }

            VAO.unBind();
        }

        void Mesh::draw(
            GLenum mode,
            const Ref<Shader>& shader,
            GLuint vao) const noexcept
        {
            glBindVertexArray(vao);

            size_t byteOffset = 0;

            for (const auto& sub : meshes)
            {
                sub->attachShader(shader);
                applyMaterial(*sub);

                VAO.draw(static_cast<uint32_t>(sub->indices.size()), mode, byteOffset);

                byteOffset += sub->indices.size() * sizeof(uint32_t);
            }

            glBindVertexArray(0);
        }


    };
};
