#ifndef SRC_RENDERER_OPENGL_UBO_HPP
#define SRC_RENDERER_OPENGL_UBO_HPP

#include <NbCore.hpp>

#include <glad/glad.h> 

#include "Renderer/IUniformBuffer.hpp"

#include <concepts>
#include <type_traits>

#include "Error/ErrorManager.hpp"

namespace nb::OpenGl
{

    template <typename T>
        requires std::is_standard_layout_v<T>
    class UniformBuffer final : public Renderer::IUniformBuffer<T>
    {
    public:
        explicit UniformBuffer(GLenum usage = GL_DYNAMIC_DRAW) noexcept
            : rendererId(0)
            , usage(usage)
        {
            glCreateBuffers(1, &rendererId);
            if (rendererId == 0)
            {
                nb::Error::ErrorManager::instance()
                    .report(nb::Error::Type::FATAL, "Failed to create OpenGL Uniform Buffer")
                    .with("usage", usage);
                return;
            }

            
            glNamedBufferData(rendererId, sizeof(T), nullptr, usage);
        }

        ~UniformBuffer() noexcept override 
        {
            if (rendererId != 0)
            {
                glDeleteBuffers(1, &rendererId);
            }
        }

        NB_NON_COPYABLE(UniformBuffer);

        UniformBuffer(UniformBuffer&& other) noexcept
            : rendererId(other.rendererId)
            , usage(other.usage)
        {
            other.rendererId = 0;
        }

        UniformBuffer& operator=(UniformBuffer&& other) noexcept
        {
            if (this != &other)
            {
                if (rendererId != 0)
                {
                    glDeleteBuffers(1, &rendererId);
                }
                rendererId       = other.rendererId;
                usage            = other.usage;
                other.rendererId = 0;
            }
            return *this;
        }

        void bind() const noexcept override
        {
            glBindBuffer(GL_UNIFORM_BUFFER, rendererId);
        }

        void unbind() const noexcept override
        {
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }

        void bindBase(GLuint bindingPoint) const noexcept override
        {
            glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, rendererId);
        }

        void updateRaw(
            const void* data,
            size_t  size
        ) noexcept override
        {
            if (rendererId == 0)
            {
                nb::Error::ErrorManager::instance().report(
                    nb::Error::Type::WARNING, "Attempted to update an uninitialized buffer"
                );
                return;
            }

            if (size > static_cast<size_t>(sizeof(T)))
            {
                nb::Error::ErrorManager::instance()
                    .report(
                        nb::Error::Type::WARNING,
                        "Update size exceeds buffer capacity. Truncating data transfer."
                    )
                    .with("requestedSize", size)
                    .with("bufferCapacity", sizeof(T));
            }

            const GLsizeiptr updateSize = (size > static_cast<GLsizeiptr>(sizeof(T)))
                                              ? static_cast<GLsizeiptr>(sizeof(T))
                                              : size;

            glNamedBufferSubData(rendererId, 0, updateSize, data);
        }

        NB_NODISCARD uint32_t getId() const noexcept override
        {
            return rendererId;
        }

        void update(const T& data) noexcept
        {
            updateRaw(&data, sizeof(T));
        }

    private:
        GLuint rendererId;
        GLenum usage;
    };

} 

#endif 