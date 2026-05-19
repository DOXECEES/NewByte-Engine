// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "Camera.hpp"

#include "Math/RayCast/RayPicker.hpp"
#include "../Debug.hpp"

#include "Scene.hpp"

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

        void Camera::updateOrbitByAngles(
            const float   yawRadians,
            const float   pitchRadians,
            Scene*        scene,
            Ecs::EntityID ballId
        ) noexcept
        {
            auto qYaw =
                Math::Quaternion<float>::axisAngleToQuaternion(-yawRadians, {0.0f, 1.0f, 0.0f});
            auto qPitch =
                Math::Quaternion<float>::axisAngleToQuaternion(-pitchRadians, {1.0f, 0.0f, 0.0f});
            totalRotation = qYaw.cross(qPitch);
            totalRotation.normalize();

            Math::Vector3<float> rayDir =
                Math::rotate(totalRotation, Math::Vector3<float>{0.0f, 0.0f, 1.0f});

            float finalDistance = distance;
            if (scene)
            {
                Math::Ray ray;
                ray.origin    = target; 
                ray.direction = rayDir;

                auto hit = scene->raycast(ray, ballId);
                if (hit.hasHit && hit.distance < distance)
                {
                    finalDistance = hit.distance - 0.3f;
                    if (finalDistance < 0.5f)
                    {
                        finalDistance = 0.5f;
                    }
                }
            }

            position  = target + (rayDir * finalDistance);
            direction = Math::normalize(target - position);

            lookAt = Math::lookAt(position, target, {0.0f, 1.0f, 0.0f});

            projection = Math::projection(
                Math::toRadians(Core::EngineSettings::getFov()),
                Core::EngineSettings::getAspectRatio(), nearPlane, farPlane
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

        float Camera::getFov() const noexcept
        {
            return Core::EngineSettings::getFov(); // TODO: Должно принадлежать камере
        }

        float Camera::getAspectRatio() const noexcept
        {
            return Core::EngineSettings::getAspectRatio(); // TODO: Должно принадлежать камере
        }

        float Camera::getNearPlane() const noexcept
        {
            return nearPlane;
        }

        void Camera::setNearPlane(float newPlane) noexcept
        {
            if (newPlane > 0.001)
            {
                nearPlane = newPlane;
            }
            else
            {
                nearPlane = NEAR_PLANE;
            }
        }

        float Camera::getFarPlane() const noexcept
        {
            return farPlane;
        }

        void Camera::setFarPlane(float newPlane) noexcept
        {
            if (newPlane > nearPlane)
            {
                farPlane = newPlane;
            }
            else
            {
                farPlane = FAR_PLANE;
            }
        }
        
        void Camera::setDistance(float dist) noexcept
        {
            distance = dist;
        }

        void Camera::setTarget(const Math::Vector3<float>& localTarget) noexcept
        {
            target = localTarget;
        }





    };
};