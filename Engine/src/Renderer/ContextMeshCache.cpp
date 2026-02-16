#include "ContextMeshCache.hpp"

namespace nb::Renderer
{
    ContextMesh* ContextMeshCache::get(
        HGLRC hglrc,
        const Mesh* mesh) noexcept
    {
        auto ctxIt = meshes.find(hglrc);
        if (ctxIt == meshes.end())
        {
            return nullptr;
        }

        auto& inner = ctxIt->second;

        auto meshIt = inner.find(mesh);
        if (meshIt == inner.end())
        {
            return nullptr;
        }

        return &meshIt->second;
    }

};

namespace nb::OpenGl
{
    Renderer::ContextMesh* OpenglContextMeshCache::insertMesh(
        HGLRC hglrc,
        Ref<Renderer::Mesh> mesh
    ) noexcept
    {
        auto& ctx = meshes[hglrc];

        auto it = ctx.find(mesh.get());
        if (it != ctx.end())
        {
            return &it->second;
        }

        GLuint vao = 0;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, mesh->getVboId());
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->getEboId());

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0, 3, GL_FLOAT, GL_FALSE,
            sizeof(Renderer::Vertex),
            (void*)0
        );

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(
            3, 2, GL_FLOAT, GL_FALSE,
            sizeof(Renderer::Vertex),
            (void*)offsetof(Renderer::Vertex, textureCoordinates)
        );

        glBindVertexArray(0);

        Renderer::ContextMesh cm{};
        cm.vao = vao;
        cm.source = mesh;

        ctx[mesh.get()] = cm;

        return &ctx[mesh.get()];
    }
};



