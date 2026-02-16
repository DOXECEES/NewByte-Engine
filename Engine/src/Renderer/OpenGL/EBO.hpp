#ifndef SRC_RENDERER_OPENGL_EBO_HPP
#define SRC_RENDERER_OPENGL_EBO_HPP

#include <NbCore.hpp>
#include <glad/glad.h>

#include "IBuffer.hpp"

#include <vector>

namespace nb
{
    namespace OpenGl
    {
        class EBO : public IBuffer
        {

        public:

            EBO() noexcept;
            ~EBO() noexcept;

            NB_COPYMOVABLE(EBO);

            //EBO(const EBO& other) noexcept = delete;
            //EBO& operator=(const EBO& other) noexcept = delete;
            //
            //EBO(EBO&& other) noexcept = delete;
            //EBO& operator=(EBO&& other) noexcept = delete;

            virtual void bind() const noexcept override;
            virtual void unBind() const noexcept override;

            void setData(const std::vector<GLuint>& data) const noexcept;

            inline const GLuint *getBuffer() const noexcept { return &buffer; };
        };
    };
};

#endif