#include "Camera.hpp"

namespace nb
{
    namespace Renderer
    {
        void Camera::update(const float newYaw, const float newPitch) noexcept
        {

            yaw = newYaw;
            pitch = newPitch;

            direction.x = std::cos(Math::toRadians(yaw)) * std::cos(Math::toRadians(pitch));
            direction.y = std::sin(Math::toRadians(pitch));
            direction.z = std::sin(Math::toRadians(yaw)) * std::cos(Math::toRadians(pitch));

            direction.normalize();
            direction = position + direction;

            lookAt = Math::lookAt(position, direction, up);
        }
    };
};