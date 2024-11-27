#ifndef SRC_MATH_RAYCAST_RAYPICKER_HPP
#define SRC_MATH_RAYCAST_RAYPICKER_HPP

#include "../../Renderer/Camera.hpp"

#include "../../Core.hpp"

namespace nb
{
    namespace Math
    {
        class RayPicker
        {
        
        public:
            RayPicker() = default;

            Math::Vector4<float> cast(Renderer::Camera* cam, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height) noexcept;

        private:

        
        };
    };
};

#endif

