// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "EBO.hpp"

nb::OpenGl::EBO::EBO() noexcept
    :IBuffer()
{
    glGenBuffers(1, &buffer);
}

nb::OpenGl::EBO::~EBO() noexcept
{
    glDeleteBuffers(1, &buffer);
}

void nb::OpenGl::EBO::bind() const noexcept 
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
}

void nb::OpenGl::EBO::unBind() const noexcept 
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void nb::OpenGl::EBO::setData(const std::vector<GLuint> &data) const noexcept
{
    bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() * sizeof(GLuint), data.data(), GL_STATIC_DRAW);
}
