#ifndef SRC_RENDERER_MESH_HPP
#define SRC_RENDERER_MESH_HPP

#include <Windows.h>

#include "RendererStructures.hpp"
#include "OpenGL/VertexArray.hpp"

#include "../Resources/IResource.hpp"

#include "../Math/AABB3D.hpp"

//
//
#include <vector>
#include <memory>


namespace nb
{
    namespace Renderer
    {


        class SubMesh
        {
            friend class Mesh;

        public:
            //SubMesh() = default;
            SubMesh(const std::vector<uint32_t> &indices, const Material& mat = {}) noexcept;
            SubMesh(const SubMesh&) = delete;
            SubMesh& operator=(const SubMesh&) = delete;

            SubMesh(SubMesh&& other) noexcept = default;
            SubMesh& operator=(SubMesh&& other) noexcept = default;

            ~SubMesh() = default;

            // SubMesh &operator=(const SubMesh &other) = default;
            // SubMesh &operator=(SubMesh &&other) noexcept = default;

            // SubMesh(const SubMesh &other) = default;
            // SubMesh(SubMesh&& other) noexcept = default;

            void attachShader(const Ref<Shader> &shader) noexcept;

            //inline const Math::AABB3D &getAABB() const noexcept { return aabb; };
            inline const Material &getMaterial() const noexcept { return material; };
           // void draw(const GLenum mode) const noexcept;
           // void drawAabb() noexcept;

       // private:
            //std::unique_ptr<std::vector<Vertex>> verticies;
            std::vector<uint32_t> indices;
            Material material;
            std::shared_ptr<ShaderUniforms> shader;

            //Math::AABB3D aabb;
            //nb::OpenGl::VertexArray VAO;

        };


        class Mesh : public Resource::IResource
        {
            public:
            
                Mesh(const std::vector<Vertex>& vert, const std::vector<uint32_t>& ind);
                explicit Mesh(std::vector<std::unique_ptr<SubMesh>> &&meshes, std::vector<Vertex>&& vertex)
                    : meshes(std::move(meshes))
                    , verticies(std::move(vertex))
                {
                    ind = uniteIndicies();
                    calculateTagnentArray();
                    
                    VAO.linkData(verticies, ind);
                    recalculateAabb3dForce();
                };

                
                std::vector<Material> getMaterials() const noexcept;
                
                
                const Math::AABB3D &getAabb3d() const noexcept;
                const Math::AABB3D &recalculateAabb3dForce() noexcept;

                void draw(GLenum mode, const Ref<Shader>& shader) const noexcept;
                
                void calculateTagnentArray()
                {
                    size_t triangleCount = indiciesCount / 3;

                    Math::Vector3<float> *tan1 = new Math::Vector3<float>[verticies.size() * 2];
                    Math::Vector3<float> *tan2 = tan1 + verticies.size();
                    ZeroMemory(tan1, verticies.size() * sizeof(Math::Vector3<float>) * 2);


                    for (size_t a = 0; a < triangleCount; a++) {
                        int i1 = ind[a * 3 + 0];
                        int i2 = ind[a * 3 + 1];
                        int i3 = ind[a * 3 + 2];
                
                        const Math::Vector3<float>& v1 = verticies[i1].position;
                        const Math::Vector3<float>& v2 = verticies[i2].position;
                        const Math::Vector3<float>& v3 = verticies[i3].position;
                
                        const Math::Vector2<float>& w1 = verticies[i1].textureCoodinates;
                        const Math::Vector2<float>& w2 = verticies[i2].textureCoodinates;
                        const Math::Vector2<float>& w3 = verticies[i3].textureCoodinates;
                
                        float x1 = v2.x - v1.x;
                        float x2 = v3.x - v1.x;
                        float y1 = v2.y - v1.y;
                        float y2 = v3.y - v1.y;
                        float z1 = v2.z - v1.z;
                        float z2 = v3.z - v1.z;
                
                        float s1 = w2.x - w1.x;
                        float s2 = w3.x - w1.x;
                        float t1 = w2.y - w1.y;
                        float t2 = w3.y - w1.y;
                
                        float r = 1.0F / (s1 * t2 - s2 * t1);
                        Math::Vector3<float> sdir(
                            (t2 * x1 - t1 * x2) * r,
                            (t2 * y1 - t1 * y2) * r,
                            (t2 * z1 - t1 * z2) * r
                        );
                        Math::Vector3<float> tdir(
                            (s1 * x2 - s2 * x1) * r,
                            (s1 * y2 - s2 * y1) * r,
                            (s1 * z2 - s2 * z1) * r
                        );

                        tan1[i1] += sdir;
                        tan1[i2] += sdir;
                        tan1[i3] += sdir;
                
                        tan2[i1] += tdir;
                        tan2[i2] += tdir;
                        tan2[i3] += tdir;
                    }
                
                    for (size_t a = 0; a < verticies.size(); a++) {
                        const Math::Vector3<float>& n = verticies[a].normal;
                        const Math::Vector3<float>& t = tan1[a];
                
                        // Gram-Schmidt orthogonalization
                        Math::Vector3<float> tangentVec = (t - n * n.dot(t));
                        tangentVec.normalize();

                        // Calculate handedness
                        float handedness = (n.cross(t).dot(tan2[a]) < 0.0F) ? -1.0F : 1.0F;
                
                        verticies[a].tangent = Math::Vector4<float>(tangentVec.x, tangentVec.y,tangentVec.z, handedness);
                    }
                }
                
                ShaderUniforms uniforms;
            
            private:
                std::vector<uint32_t> uniteIndicies() noexcept;
                void applyMaterial(const SubMesh& mesh) const noexcept;

            //private:


                Math::AABB3D aabb;
                nb::OpenGl::VertexArray VAO;

                std::vector<uint32_t> ind;
                std::vector<Vertex> verticies;
                std::vector<std::unique_ptr<SubMesh>> meshes;
                size_t indiciesCount = 0;
        };
    };
};

#endif