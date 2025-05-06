#ifndef SRC_RENDERER_OPENGL_VERTEXARRAY_HPP
#define SRC_RENDERER_OPENGL_VERTEXARRAY_HPP

#include <glad/glad.h>

#include "../RendererStructures.hpp"

#include "VBO.hpp"
#include "EBO.hpp"

#include <vector>

namespace nb
{
    namespace OpenGl
    {
        class VertexArray
        {
            
        public:

            VertexArray() noexcept;
            ~VertexArray() noexcept;

            void bind() const noexcept;
            void unBind() const noexcept;

            void linkData(const std::vector<nb::Renderer::Vertex> &vert, const std::vector<GLuint> &ind);

            void draw(const size_t count, GLenum modem, const size_t offset = 0) const noexcept;
            
            

        private:

            VBO     vbo;
            EBO     ebo;
            GLuint  array = 0;
        
        };
        
        
        

    };
};

#endif