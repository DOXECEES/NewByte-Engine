#include "Camera.hpp"

#include "../Debug.hpp"


namespace nb
{
    namespace Renderer
    {
        void Camera::update(const float newYaw, const float newPitch) noexcept
        {
            float deltaYaw = (yaw - newYaw) * 0.003f;
            float deltaPitch = (pitch - newPitch) * 0.003f;

            yaw = newYaw;
            pitch = newPitch;

            if (alignByX)
            {
                deltaYaw = 0.0f;
                direction.x = 0.0f;
            }
            if (alignByY)
            {
                deltaPitch = 0.0f;
                direction.y = 0.0f;
            }

            auto rightDirection = direction.cross(up);

            if(deltaYaw != 0.0f || deltaPitch != 0.0f)
            {
                auto yawQuat = Math::Quaternion<float>::axisAngleToQuaternion(deltaYaw, up);
                auto pitchQuat = Math::Quaternion<float>::axisAngleToQuaternion(-deltaPitch, rightDirection);

                auto q = yawQuat.cross(pitchQuat);
            
                q.normalize();

                direction = Math::rotate(q, direction);
            }

           

            lookAt = Math::lookAt(position, position + direction, up);

            projection = Math::projection(
                Math::toRadians(Core::EngineSettings::getFov()),
                Core::EngineSettings::getAspectRatio(),
                NEAR_PLANE,
                FAR_PLANE);
        }
    };
};