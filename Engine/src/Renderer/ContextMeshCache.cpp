// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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

        glVertexAttribPointer(
            0, 3, GL_FLOAT, GL_FALSE, sizeof(nb::Renderer::Vertex),
            reinterpret_cast<void*>(0 + offsetof(nb::Renderer::Vertex, position))
        );
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(
            1, 3, GL_FLOAT, GL_FALSE, sizeof(nb::Renderer::Vertex),
            reinterpret_cast<void*>(0 + offsetof(nb::Renderer::Vertex, normal))
        );
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(
            2, 3, GL_FLOAT, GL_FALSE, sizeof(nb::Renderer::Vertex),
            reinterpret_cast<void*>(0 + offsetof(nb::Renderer::Vertex, color))
        );
        glEnableVertexAttribArray(2);

        glVertexAttribPointer(
            3, 2, GL_FLOAT, GL_FALSE, sizeof(nb::Renderer::Vertex),
            reinterpret_cast<void*>(0 + offsetof(nb::Renderer::Vertex, textureCoordinates))
        );
        glEnableVertexAttribArray(3);

        glVertexAttribPointer(
            4, 4, GL_FLOAT, GL_FALSE, sizeof(nb::Renderer::Vertex),
            reinterpret_cast<void*>(0 + offsetof(nb::Renderer::Vertex, tangent))
        );
        glEnableVertexAttribArray(4);

        glBindVertexArray(0);

        Renderer::ContextMesh cm{};
        cm.vao = vao;
        cm.source = mesh;

        ctx[mesh.get()] = cm;

        return &ctx[mesh.get()];
    }
};



