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

void nb::OpenGl::VertexArray::linkData(const std::vector<nb::OpenGl::VBO::Vertex> &vert, const std::vector<GLuint> &ind)
{
    bind();
    //vbo.bind();
    vbo.setData(vert);

    //ebo.bind();
    ebo.setData(ind);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(nb::OpenGl::VBO::Vertex), reinterpret_cast<void*>(0 + offsetof(nb::OpenGl::VBO::Vertex, position)));
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // glEnableVertexAttribArray(1);
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(nb::OpenGl::VBO::Vertex), reinterpret_cast<void*>(0 + offsetof(nb::OpenGl::VBO::Vertex, normal)));
    
    // glEnableVertexAttribArray(2);
    // glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(nb::OpenGl::VBO::Vertex), reinterpret_cast<void*>(0 + offsetof(nb::OpenGl::VBO::Vertex, color)));
        
    // glEnableVertexAttribArray(3);
    // glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(nb::OpenGl::VBO::Vertex), reinterpret_cast<void*>(0 + offsetof(nb::OpenGl::VBO::Vertex, texCoords)));

    vbo.unBind();

    glBindVertexArray(0);
}

void nb::OpenGl::VertexArray::draw() const noexcept
{
    for (size_t i = 0; i < 4; i++)
    {
        glEnableVertexAttribArray(i);
    }

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    for (size_t i = 0; i < 4; i++)
    {
        glDisableVertexAttribArray(i);
    }
}
