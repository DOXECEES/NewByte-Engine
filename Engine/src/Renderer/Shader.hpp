#ifndef SRC_RENDERER_SHADER_HPP
#define SRC_RENDERER_SHADER_HPP

#include "../Resources/IResource.hpp"

#include "../Math/Vector2.hpp"
#include "../Math/Vector3.hpp"
#include "../Math/Vector4.hpp"

#include "../Math/Matrix/Matrix.hpp"

namespace nb
{
    namespace Renderer
    {
        class Shader : public Resource::IResource
        {
        public:
            Shader() noexcept;
            virtual ~Shader() noexcept = default;

            virtual void use() noexcept = 0;

            virtual void setUniformFloat(std::string_view name, const float value) const noexcept = 0;
            virtual void setUniformInt(std::string_view name, const int value) const noexcept = 0;

            virtual void setUniformVec2(std::string_view name, const Math::Vector2<float> value) const noexcept = 0;
            virtual void setUniformVec3(std::string_view name, const Math::Vector3<float> value) const noexcept = 0;
            virtual void setUniformVec4(std::string_view name, const Math::Vector4<float> value) const noexcept = 0;

            virtual void setUniformMat2(std::string_view name, const Math::Mat2<float>& value) const noexcept = 0;
            virtual void setUniformMat3(std::string_view name, const Math::Mat3<float>& value) const noexcept = 0;
            virtual void setUniformMat4(std::string_view name, const Math::Mat4<float>& value) const noexcept = 0;

            inline int getId() const noexcept { return id; };

        private:
            int id;

            static int globalId;
        };
    };
};

#endif
