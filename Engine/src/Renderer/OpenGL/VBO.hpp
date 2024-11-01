#ifndef SRC_RENDERER_OPENGL_VBO_HPP
#define SRC_RENDERER_OPENGL_VBO_HPP

#include <glad/glad.h>

#include "../../Math/Vector2.hpp"
#include "../../Math/Vector3.hpp"

#include "IBuffer.hpp"

#include <vector>

namespace nb
{
    namespace OpenGl
    {
        class VBO : public IBuffer
        {

        public:

            struct Vertex
            {
                Math::Vector3<float> position;
                Math::Vector3<float> normal;
                Math::Vector3<float> color;
                Math::Vector2<float> texCoords;
            };

            VBO() noexcept;
            ~VBO() noexcept;

            VBO(const VBO& other) noexcept = delete;
            VBO& operator=(const VBO& other) noexcept = delete;

            VBO(VBO&& other) noexcept = delete;
            VBO& operator=(VBO&& other) noexcept = delete;

            virtual void bind() const noexcept override;
            virtual void unBind() const noexcept override;

            void setData(const std::vector<Vertex>& data) const noexcept;



        };
    };
};

#endif