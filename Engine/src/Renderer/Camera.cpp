#include "Camera.hpp"
#include "../Debug.hpp"

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
            Debug::debug(direction.x);
            Debug::debug(direction.y);
            Debug::debug(direction.z);

            lookAt = Math::lookAt(position, position + direction, up);
        }
    };
};