#ifndef SRC_RENDERER_CAMERA_HPP
#define SRC_RENDERER_CAMERA_HPP

#include <cmath>

#include "../Math/Math.hpp"

#include "../Math/Quaternion.hpp"

#include "../Math/Vector3.hpp"
#include "../Math/Matrix/Matrix.hpp"
#include "../Math/Matrix/Transformation.hpp"

#include "../Core/EngineSettings.hpp"

namespace nb
{
    namespace Renderer
    {
        class Camera
        {
        public:
            Camera() = default;
            ~Camera() = default;

            inline void moveTo(const Math::Vector3<float> &newPos) noexcept { position = newPos; };

            void toggleAlignByX() noexcept { alignByX = !alignByX; };
            void toggleAlignByY() noexcept { alignByY = !alignByY; };
            void toggleAlignByZ() noexcept { alignByZ = !alignByZ; };

            inline const Math::Mat4<float> &getLookAt() const noexcept { return lookAt; };
            inline const Math::Mat4<float> &getProjection() const noexcept { return projection; };
            void update(const float newYaw, const float newPitch) noexcept;



            inline Math::Vector3<float> getPosition() const noexcept { return position; };
            inline Math::Vector3<float> getDirection() const noexcept { return direction; };
            

            //Math::Mat4<T> getViewMatrix() const {
               
            //}

        private:


            Math::Mat4<float> lookAt;
            Math::Mat4<float> projection;

            bool alignByX = false;
            bool alignByY = false;
            bool alignByZ = false;

            Math::Vector3<float> position   = { 0.0f, 0.0f, 0.0f };
            Math::Vector3<float> direction  = { 0.0f, 0.0f, -1.0f };
            Math::Vector3<float> up         = { 0.0f, 1.0f, 0.0f };
            float yaw                       =   0.0f;
            float pitch                     =   0.0f;

        };
    };
};

#endif

