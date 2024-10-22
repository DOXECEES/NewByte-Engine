#ifndef SRC_RENDERER_OPENGL_SHADER_HPP
#define SRC_RENDERER_OPENGL_SHADER_HPP

#include <glad/glad.h>

#include <filesystem>
#include <vector>

#include <fstream>
#include <string>
#include <sstream>

// TODO 2: (after) delete
#include "../../Debug.hpp"
#include "../../Fatal.hpp"
// TODO 3: make pch for NbCommon (also rename NbCommon to Core.hpp (CMAke));

namespace nb
{
    namespace OpenGl
    {
        class Shader
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

            Shader() noexcept = default;
            explicit Shader(const std::filesystem::path &pathToShader) noexcept;
            explicit Shader(const std::vector<std::filesystem::path> &vecOfShaders) noexcept;
            ~Shader() noexcept;

            void link(const std::filesystem::path &pathToShader) noexcept;
            void use() noexcept;
            void setUniform(const std::string &name, const float value) const noexcept;


        private:

            // bool isValid() const noexcept;
            void load(const std::filesystem::path &pathToShader) noexcept;
            bool isCompiled() const noexcept;
            ShaderType getShaderType(const std::filesystem::path& path) const noexcept;

            // temp
            std::string loadFromFile(const std::filesystem::path &path) noexcept;

        private:

            std::vector<GLuint> shaders;
            GLuint program = 0;

        };
    };
};

#endif
