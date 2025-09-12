// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "RayPicker.hpp"

namespace nb
{
    namespace Math
    {
        Math::Vector3<float> RayPicker::cast(Renderer::Camera* cam, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height) noexcept
        {
            double xn = (2.0 * (double)x) / (double)width - 1.0;
            double yn = 1.0 - (2.0 * (double)y) / (double)height;

            Math::Vector4<float> clipSpace = {(float)xn, (float)yn, -1.0f, 1.0f};

            auto proj = cam->getProjection();

            auto eyeSpace = Math::inverse(proj) * clipSpace;
            eyeSpace.z = -1.0;
            eyeSpace.w = 0.0;

            auto view = cam->getLookAt();

            auto invM = Math::inverse(view);
            auto worldSpace = invM * eyeSpace;  
            worldSpace.normalize(); 
            
            return Math::Vector3<float>(worldSpace.x, worldSpace.y, worldSpace.z);
        }
    };
};