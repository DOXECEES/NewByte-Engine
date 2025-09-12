// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "VBO.hpp"

nb::OpenGl::VBO::VBO() noexcept
    :IBuffer()
{
    glGenBuffers(1, &buffer);
}

nb::OpenGl::VBO::~VBO() noexcept
{
    glDeleteBuffers(1, &buffer);
}

void nb::OpenGl::VBO::bind() const noexcept
{
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
}

void nb::OpenGl::VBO::unBind() const noexcept
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void nb::OpenGl::VBO::setData(const std::vector<nb::Renderer::Vertex> &data) const noexcept
{
    bind();
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(nb::Renderer::Vertex), data.data(), GL_STATIC_DRAW);
}
