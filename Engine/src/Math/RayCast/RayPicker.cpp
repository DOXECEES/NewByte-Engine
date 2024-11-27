#include "RayPicker.hpp"

namespace nb
{
    namespace Math
    {
        Math::Vector4<float> RayPicker::cast(Renderer::Camera* cam, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height) noexcept
        {
            float xn = float((2.0f * (float)x) / (float)width) - 1.0f;
            float yn = float((2.0f * (float)y) / (float)height) - 1.0f;
            float zn = 1.0f;

            auto proj = cam->getProjection();
            auto view = cam->getLookAt();

            Vector4<float> clipSpace = {xn, yn, -1.0, 1.0};
            auto eyeSpace  = Math::inverse(proj) * clipSpace;
            eyeSpace.z = -1.0;
            eyeSpace.w = 0.0; 

            Vector4<float> worldSpace= Math::inverse(view) * eyeSpace;
            
            //vinvP.normalize();
            return worldSpace;
        }
    };
};