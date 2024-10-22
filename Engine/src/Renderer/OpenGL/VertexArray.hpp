#ifndef SRC_RENDERER_OPENGL_VERTEXARRAY_HPP
#define SRC_RENDERER_OPENGL_VERTEXARRAY_HPP

#include <glad/glad.h>


#include "VBO.hpp"
#include "EBO.hpp"

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

            void linkData(const std::vector<VBO::Vertex> &vert, const std::vector<GLuint> &ind);

            void draw() const noexcept;
            
            

        private:

            VBO vbo;
            EBO ebo;
            GLuint array = 0;
        
        };
        
        
        

    };
};

#endif