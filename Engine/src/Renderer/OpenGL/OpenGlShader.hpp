#ifndef SRC_RENDERER_OPENGL_SHADER_HPP
#define SRC_RENDERER_OPENGL_SHADER_HPP

#include <glad/glad.h>

#include <filesystem>
#include <vector>

#include <fstream>
#include <string>
#include <sstream>

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
            explicit OpenGlShader(const std::filesystem::path &pathToShader) noexcept;
            explicit OpenGlShader(const std::vector<std::filesystem::path> &vecOfShaders) noexcept;
            ~OpenGlShader() noexcept;

            void recompile() noexcept;

            void link(const std::filesystem::path &pathToShader) noexcept;
            void use() noexcept override;

            virtual void setUniformFloat(std::string_view name, const float value) const noexcept override final;
            virtual void setUniformInt(std::string_view name, const int value) const noexcept override final;

            virtual void setUniformVec2(std::string_view name, const Math::Vector2<float> value) const noexcept override final;
            virtual void setUniformVec3(std::string_view name, const Math::Vector3<float> value) const noexcept override final;
            virtual void setUniformVec4(std::string_view name, const Math::Vector4<float> value) const noexcept override final;

            virtual void setUniformMat2(std::string_view name, const Math::Mat2<float>& value) const noexcept override final;
            virtual void setUniformMat3(std::string_view name, const Math::Mat3<float>& value) const noexcept override final;
            virtual void setUniformMat4(std::string_view name, const Math::Mat4<float>& value) const noexcept override final;



        private:
            void createProgram() noexcept;
            void load(const std::filesystem::path &pathToShader) noexcept;
            bool isCompiled() const noexcept;
            ShaderType getShaderType(const std::filesystem::path& path) const noexcept;

            // temp
            std::string loadFromFile(const std::filesystem::path &path) noexcept;

        private:

            std::vector<GLuint>                 shaders;
            std::vector<std::filesystem::path>  pathsToShaderSources;
            GLuint                              program                 = 0;

        };
    };
};

#endif
