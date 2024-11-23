#ifndef SRC_RENDERER_OPENGL_FBO_HPP
#define SRC_RENDERER_OPENGL_FBO_HPP

#include "IBuffer.hpp"

namespace nb
{
    namespace OpenGl
    {
        class FBO : public IBuffer 
        {
        
        public:
            FBO() noexcept;
            ~FBO() noexcept;

            FBO(const FBO& other) noexcept = delete;
            FBO& operator=(const FBO& other) noexcept = delete;

            FBO(FBO&& other) noexcept = delete;
            FBO& operator=(FBO&& other) noexcept = delete;

            virtual void bind() const noexcept override;
            virtual void unBind() const noexcept override;

        };
    };

};

#endif