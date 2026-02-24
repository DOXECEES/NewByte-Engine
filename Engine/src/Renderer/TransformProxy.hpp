#ifndef SRC_RENDERER_TRANSFORMPROXY_HPP
#define SRC_RENDERER_TRANSFORMPROXY_HPP

#include "SceneGraph.hpp"

namespace nb::Renderer
{
    class TransformProxy
    {
    public:
        TransformProxy(
            BaseNode& owner,
            Transform& transform
        )
            : owner(owner),
              transform(transform)
        {}

        float& translateX() noexcept
        {
            owner.markDirty();
            return transform.translate.x;
        }

        float& translateY() noexcept
        {
            owner.markDirty();
            return transform.translate.y;
        }

        float& translateZ() noexcept
        {
            owner.markDirty();
            return transform.translate.z;
        }

        Math::Vector3<float> getTranslate() const noexcept
        {
            return transform.translate;
        }

        void setTranslate(const Math::Vector3<float>& v) noexcept
        {
            owner.markDirty();
            transform.translate = v;
        }

    private:

        BaseNode& owner;
        Transform& transform;
    };

} // namespace nb::Renderer

#endif