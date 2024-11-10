#include "Camera.hpp"

namespace nb
{
    namespace Renderer
    {
        void Camera::update(const float newYaw, const float newPitch) noexcept
        {
            yaw = newYaw;
            pitch = newPitch;

            direction.x = -std::cos(yaw) * std::cos(pitch);
            direction.y = std::sin(pitch);
            direction.z = std::sin(yaw) * std::cos(pitch);

            direction.normalize();
            direction = position + direction;

            lookAt = Math::lookAt(position, direction, up);
        }
    };
};