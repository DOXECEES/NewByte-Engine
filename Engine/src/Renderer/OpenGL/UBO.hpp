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
    class UniformBuffer final : public Renderer::IUniformBufferBase
    {
    public:
        // Теперь конструктор принимает размер буфера напрямую
        explicit UniformBuffer(size_t size, GLenum usage = GL_DYNAMIC_DRAW) noexcept
            : rendererId(0)
            , usage(usage)
            , m_size(size)
        {
            glCreateBuffers(1, &rendererId);
            if (rendererId == 0)
            {
                nb::Error::ErrorManager::instance()
                    .report(nb::Error::Type::FATAL, "Failed to create OpenGL Uniform Buffer")
                    .with("usage", usage);
                return;
            }
            glNamedBufferData(rendererId, size, nullptr, usage);
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
            , m_size(other.m_size)
        {
            other.rendererId = 0;
            other.m_size = 0;
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
                m_size           = other.m_size;
                other.rendererId = 0;
                other.m_size     = 0;
            }
            return *this;
        }

        void bind() const noexcept override { glBindBuffer(GL_UNIFORM_BUFFER, rendererId); }
        void unbind() const noexcept override { glBindBuffer(GL_UNIFORM_BUFFER, 0); }
        void bindBase(uint32_t bindingPoint) const noexcept override 
        { 
            glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, rendererId); 
        }

        void updateRaw(const void* data, size_t size) noexcept override
        {
            if (rendererId == 0) return;

            if (size > m_size)
            {
                nb::Error::ErrorManager::instance()
                    .report(nb::Error::Type::WARNING, "Update size exceeds buffer capacity.")
                    .with("requestedSize", size)
                    .with("bufferCapacity", m_size);
            }

            const GLsizeiptr updateSize = (size > m_size) ? static_cast<GLsizeiptr>(m_size) : size;
            glNamedBufferSubData(rendererId, 0, updateSize, data);
        }

        NB_NODISCARD uint32_t getId() const noexcept override { return rendererId; }

    private:
        GLuint rendererId;
        GLenum usage;
        size_t m_size;
    };
}

#endif 