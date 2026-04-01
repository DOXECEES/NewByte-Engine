// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "Camera.hpp"

#include "Math/RayCast/RayPicker.hpp"
#include "../Debug.hpp"



namespace nb
{
    namespace Renderer
    {
        void Camera::updateOrbit(
            const float deltaX,
            const float deltaY
        ) noexcept
        {
            float sensitivity = 0.003f;

            Math::Vector3<float> cameraRight = Math::rotate(totalRotation, Math::Vector3<float>{1.0f, 0.0f, 0.0f});
            Math::Vector3<float> cameraUp = Math::rotate(totalRotation, Math::Vector3<float>{0.0f, 1.0f, 0.0f});

            auto qX = Math::Quaternion<float>::axisAngleToQuaternion(-deltaX * sensitivity, cameraUp);
            auto qY = Math::Quaternion<float>::axisAngleToQuaternion(-deltaY * sensitivity, cameraRight);

            auto deltaRotation = qX.cross(qY); 
            totalRotation = deltaRotation.cross(totalRotation);
            totalRotation.normalize();

            Math::Vector3<float> startOffset(0.0f, 0.0f, distance);
            Math::Vector3<float> offset = Math::rotate(totalRotation, startOffset);

            position = target + offset;

            direction = (target - position);
            direction.normalize();

            Math::Vector3<float> upVector =
                Math::rotate(totalRotation, Math::Vector3<float>{0.0f, 1.0f, 0.0f});
            lookAt = Math::lookAt(position, target, upVector);

            projection = Math::projection(
                Math::toRadians(Core::EngineSettings::getFov()),
                Core::EngineSettings::getAspectRatio(), NEAR_PLANE, FAR_PLANE
            );
        }

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


            if(deltaYaw != 0.0f || deltaPitch != 0.0f)
            {
                auto rightDirection = direction.cross(up);

                auto yawQuat = Math::Quaternion<float>::axisAngleToQuaternion(deltaYaw, up);
                auto pitchQuat = Math::Quaternion<float>::axisAngleToQuaternion(deltaPitch, rightDirection);

                auto q = yawQuat.cross(pitchQuat);
            
                q.normalize();

                direction = Math::rotate(q, direction);
            }

            lookAt = Math::lookAt(position, position + direction, up);

            projection = Math::projection(
                Math::toRadians(Core::EngineSettings::getFov()),
                Core::EngineSettings::getAspectRatio(),
                NEAR_PLANE,
                FAR_PLANE
            );
        }

        Math::Ray Camera::getRayFromMousePoint(
            uint32_t x,
            uint32_t y
        ) noexcept
        {
            Math::RayPicker rayPicker;
            return rayPicker.cast(
                this, x, y, Core::EngineSettings::getWidth(), Core::EngineSettings::getHeight()
            ); 
        }
    };
};