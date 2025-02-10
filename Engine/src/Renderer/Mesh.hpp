#ifndef SRC_RENDERER_MESH_HPP
#define SRC_RENDERER_MESH_HPP

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
                    std::vector<uint32_t> ind = uniteIndicies();
                    VAO.linkData(verticies, ind);
                };

                std::vector<Material> getMaterials() const noexcept;

                void draw(GLenum mode, const Ref<Shader>& shader) const noexcept;
            
            private:
                std::vector<uint32_t> uniteIndicies() noexcept;
                void applyMaterial(const SubMesh& mesh) const noexcept;

                // private:

                nb::OpenGl::VertexArray VAO;

                std::vector<Vertex> verticies;
                std::vector<std::unique_ptr<SubMesh>> meshes;
                size_t indiciesCount = 0;
        };
    };
};

#endif