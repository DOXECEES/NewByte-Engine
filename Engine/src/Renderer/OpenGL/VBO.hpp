#ifndef SRC_RENDERER_OPENGL_VBO_HPP
#define SRC_RENDERER_OPENGL_VBO_HPP

#include <NbCore.hpp>
#include <glad/glad.h>

#include "../RendererStructures.hpp"

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
            
            VBO() noexcept;
            ~VBO() noexcept;

            NB_COPYMOVABLE(VBO);

            //VBO(const VBO& other) noexcept = delete;
            //VBO& operator=(const VBO& other) noexcept = delete;
            //
            //VBO(VBO&& other) noexcept = delete;
            //VBO& operator=(VBO&& other) noexcept = delete;

            virtual void bind() const noexcept override;
            virtual void unBind() const noexcept override;

            void setData(const std::vector<nb::Renderer::Vertex>& data) const noexcept;



        };
    };
};

#endif