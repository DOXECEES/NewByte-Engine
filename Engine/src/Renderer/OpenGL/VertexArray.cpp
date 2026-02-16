// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "VertexArray.hpp"

nb::OpenGl::VertexArray::VertexArray() noexcept
{
    glGenVertexArrays(1, &array);
}

nb::OpenGl::VertexArray::~VertexArray() noexcept
{
    glDeleteVertexArrays(1, &array);
}

void nb::OpenGl::VertexArray::bind() const noexcept
{
    glBindVertexArray(array);
}

void nb::OpenGl::VertexArray::unBind() const noexcept
{
    glBindVertexArray(0);
}

void nb::OpenGl::VertexArray::linkData(const std::vector<nb::Renderer::Vertex> &vert, const std::vector<GLuint> &ind)
{
    bind();
    //vbo.bind();
    vbo.setData(vert);

    //ebo.bind();
    ebo.setData(ind);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(nb::Renderer::Vertex), reinterpret_cast<void*>(0 + offsetof(nb::Renderer::Vertex, position)));
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(nb::Renderer::Vertex), reinterpret_cast<void*>(0 + offsetof(nb::Renderer::Vertex, normal)));
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(nb::Renderer::Vertex), reinterpret_cast<void*>(0 + offsetof(nb::Renderer::Vertex, color)));
    glEnableVertexAttribArray(2);
        
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(nb::Renderer::Vertex), reinterpret_cast<void*>(0 + offsetof(nb::Renderer::Vertex, textureCoordinates)));
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(nb::Renderer::Vertex), reinterpret_cast<void*>(0 + offsetof(nb::Renderer::Vertex, tangent)));
    glEnableVertexAttribArray(4);

    vbo.unBind();

    glBindVertexArray(0);
}

void nb::OpenGl::VertexArray::draw(const size_t count, GLenum mode, const size_t offset) const noexcept
{
    for (size_t i = 0; i < 16; i++)
    {
        glEnableVertexAttribArray(i);
    }

    glDrawElements(mode, count, GL_UNSIGNED_INT, reinterpret_cast<void*>(sizeof(uint32_t) * offset));

    for (size_t i = 0; i < 16; i++)
    {
        glDisableVertexAttribArray(i);
    }
}

const nb::OpenGl::VBO& nb::OpenGl::VertexArray::getVbo() const noexcept
{
    return vbo;
}

const nb::OpenGl::EBO& nb::OpenGl::VertexArray::getEbo() const noexcept
{
    return ebo;
}
