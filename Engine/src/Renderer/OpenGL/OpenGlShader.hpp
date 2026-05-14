#ifndef SRC_RENDERER_OPENGL_SHADER_HPP
#define SRC_RENDERER_OPENGL_SHADER_HPP

#include <glad/glad.h>

#include <filesystem>
#include <vector>

#include <fstream>
#include <string>
#include <sstream>
#include <variant>
#include <Span.hpp>

#include "../../Resources/IResource.hpp"
#include "../Shader.hpp"

// TODO 2: (after) delete
#include "../../Debug.hpp"
// TODO 3: make pch for NbCommon (also rename NbCommon to Core.hpp (CMAke));


namespace nb
{
    namespace OpenGl
    {
        class OpenGlShader : public Renderer::Shader
        {

        public:
            enum class ShaderType
            {
                UNKNOWN                 = 0, 
                VERTEX                  = GL_VERTEX_SHADER,
                FRAGMENT                = GL_FRAGMENT_SHADER,
                GEOMETRY                = GL_GEOMETRY_SHADER,
                TESSELLATION_CONTROLL   = GL_TESS_CONTROL_SHADER,
                TESSELLATION_EVALUATION = GL_TESS_EVALUATION_SHADER,
                COMPUTE                 = GL_COMPUTE_SHADER,

            };

            OpenGlShader() noexcept = default;
            explicit OpenGlShader(
                const std::filesystem::path& pathToShader,
                const std::filesystem::path& shaderProgramPath,
                nbstl::Span<std::string>     params = {}
            ) noexcept;
            explicit OpenGlShader(
                const std::vector<std::filesystem::path>& vecOfShaders,
                const std::filesystem::path& shaderProgramPath,
                nbstl::Span<std::string>                  params = {}
            ) noexcept;
            ~OpenGlShader() noexcept;

            void recompile() noexcept override;

            void link(const std::filesystem::path &pathToShader) noexcept;
            void use() noexcept override;


            virtual void setUniformBool(std::string_view name, const bool value) const noexcept override final;

            virtual void setUniformFloat(std::string_view name, const float value) const noexcept override final;
            virtual void setUniformInt(std::string_view name, const int value) const noexcept override final;

             void setUniformUint64(
                std::string_view name,
                const uint64_t   value
            ) const noexcept override final;


            virtual void setUniformVec2(
                std::string_view name,
                const Math::Vector2<float>& value
            ) const noexcept override final;

            virtual void setUniformVec3(
                std::string_view name,
                const Math::Vector3<float>& value
            ) const noexcept override final;
            
            virtual void setUniformVec4(
                std::string_view name,
                const Math::Vector4<float>& value
            ) const noexcept override final;

            virtual void setUniformMat2(std::string_view name, const Math::Mat2<float>& value) const noexcept override final;
            virtual void setUniformMat3(std::string_view name, const Math::Mat3<float>& value) const noexcept override final;
            virtual void setUniformMat4(std::string_view name, const Math::Mat4<float>& value) const noexcept override final;

            void setUniformVec3Array(
                const std::string&          name,
                const Math::Vector3<float>* values,
                uint32                      count
            ) const noexcept override;

        private:
            void reapplyUniforms() const noexcept;

            void createProgram() noexcept;
            void load(const std::filesystem::path &pathToShader) noexcept;
            bool isCompiled() const noexcept;
            ShaderType getShaderType(const std::filesystem::path& path) const noexcept;

            // temp
            std::string loadFromFile(const std::filesystem::path &path) noexcept;

        private:

            using UniformValue = std::variant<
                float,
                int,
                uint64_t,
                Math::Vector2<float>,
                Math::Vector3<float>,
                Math::Vector4<float>,
                Math::Mat2<float>,
                Math::Mat3<float>,
                Math::Mat4<float>>;

            mutable std::unordered_map<std::string, UniformValue> uniformCache;

            std::vector<GLuint>                 shaders;
            std::vector<std::filesystem::path>  pathsToShaderSources;
            std::vector<std::string>            shaderParams;
            GLuint                              program                 = 0;

        };
    };
};

#endif
