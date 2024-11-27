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

            auto rightDirection = direction.cross(up);

            if(deltaYaw != 0.0f || deltaPitch != 0.0f)
            {
                auto pitchQuat = Math::Quaternion<float>::axisAngleToQuaternion(-deltaPitch, rightDirection);
                auto yawQuat = Math::Quaternion<float>::axisAngleToQuaternion(deltaYaw, up);

                auto q = pitchQuat.cross(yawQuat);
            
                q.normalize();

                direction = Math::rotate(q, direction);
            }

            //direction.normalize();

            // if (alignByX)
            // {
            //     direction.y = 0.f;
            //     direction.z = 0.f;
            // }
            // else if (alignByY)
            // {
            //     direction.x = 0.f;
            //     direction.z = 0.f;
            // }
            // else if (alignByZ)
            // {
            //     direction.x = 0.f;
            //     direction.y = 0.f;
            // }

            lookAt = Math::lookAt(position, position + direction, up);

            projection = Math::projection(
                Math::toRadians(Core::EngineSettings::getFov()),
                Core::EngineSettings::getAspectRatio(),
                1.0f,
                100.0f);
        }
    };
};