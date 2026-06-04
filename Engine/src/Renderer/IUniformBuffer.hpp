#ifndef SRC_RENDERER_IUNIFORMBUFFER_HPP
#define SRC_RENDERER_IUNIFORMBUFFER_HPP

#include <NbCore.hpp>

#include <concepts>

namespace nb::Renderer
{
    template <typename T>
        requires std::is_standard_layout_v<T>
    class IUniformBuffer
    {
    public:
        virtual ~IUniformBuffer() noexcept = default;

        virtual void bind() const noexcept                        = 0;
        virtual void unbind() const noexcept                      = 0;
        virtual void bindBase(uint32_t bindingPoint) const noexcept = 0;
        virtual void updateRaw(
            const void* data,
            size_t  size
        ) noexcept                                                  = 0;
        virtual void                 update(const T& data) noexcept = 0;
        NB_NODISCARD virtual uint32_t getId() const noexcept         = 0;
    };
}

#endif