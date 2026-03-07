// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "RayPicker.hpp"

namespace nb
{
    namespace Math
    {
        Ray RayPicker::cast(
            Renderer::Camera* cam,
            const uint32_t x,
            const uint32_t y,
            const uint32_t width,
            const uint32_t height
        ) noexcept
        {
            float xn = (2.0f * (float)x) / (float)width - 1.0f;
            float yn = 1.0f - (2.0f * (float)y) / (float)height;

            auto invVP = Math::inverse(cam->getLookAt() * cam->getProjection());

            // Точка на ближней плоскости (Z = -1)
            nb::Math::Vector4<float> nearNDC(xn, yn, -1.0f, 1.0f);
            auto nearWorld4 = invVP * nearNDC;
            nb::Math::Vector3<float> nearWorld(
                nearWorld4.x / nearWorld4.w, nearWorld4.y / nearWorld4.w,
                nearWorld4.z / nearWorld4.w
            );

            // Точка на дальней плоскости (Z = 1)
            nb::Math::Vector4<float> farNDC(xn, yn, 1.0f, 1.0f);
            auto farWorld4 = invVP * farNDC;
            nb::Math::Vector3<float> farWorld(
                farWorld4.x / farWorld4.w, farWorld4.y / farWorld4.w, farWorld4.z / farWorld4.w
            );

            Ray ray;
            ray.origin = nearWorld; // Это ГАРАНТИРОВАННО будет текущая позиция камеры в мире
            ray.direction = (farWorld - nearWorld);
            ray.direction.normalize();
            return ray;
        }

    }; // namespace Math
}; // namespace nb