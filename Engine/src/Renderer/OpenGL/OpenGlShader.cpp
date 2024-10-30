#include "OpenGlShader.hpp"

nb::OpenGl::OpenGlShader::OpenGlShader(const std::filesystem::path &pathToShader) noexcept
{
    link(pathToShader);
}

nb::OpenGl::OpenGlShader::OpenGlShader(const std::vector<std::filesystem::path> &vecOfShaders) noexcept
{
    for (const auto &shaderPath : vecOfShaders)
    {
        link(shaderPath);
    }
}

nb::OpenGl::OpenGlShader::~OpenGlShader() noexcept
{
    for (auto &i : shaders)
    {
        glDeleteShader(i);
    }
}


void nb::OpenGl::OpenGlShader::link(const std::filesystem::path &pathToShader) noexcept
{
    if (auto shaderType = getShaderType(pathToShader); shaderType != ShaderType::UNKNOWN)
    {
        shaders.push_back(glCreateShader(static_cast<GLenum>(shaderType)));
        load(pathToShader);
    }
    else
    {
        nb::Fatal::exit(L"Shader type not supported");
    }
}

void nb::OpenGl::OpenGlShader::use() noexcept
{
    if (program == 0)
    {
        program = glCreateProgram();

        for (const auto &i : shaders)
        {
            glAttachShader(program, i);
        }

        glLinkProgram(program);

        GLint isLinked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked);

        if (isLinked == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

            // The maxLength includes the NULL character
            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

            // We don't need the program anymore.
            glDeleteProgram(program);
            // Don't leak shaders either.
            for(auto &i : shaders)
            {
                glDeleteShader(i);
            }
            Debug::debug(infoLog);
            nb::Fatal::exit(L"Cannot link shader");
        }

        for (const auto &i : shaders)
        {
            glDetachShader(program, i);
        }
    }
    else
        return glUseProgram(program);
}

void nb::OpenGl::OpenGlShader::load(const std::filesystem::path &pathToShader) noexcept
{
    const char *shaderSource = loadFromFile(pathToShader).c_str();
    glShaderSource(shaders.back(), 1, &shaderSource, NULL);

    if (!isCompiled())
    {
        nb::Fatal::exit(L"Cannot compile shader");
    }
}

bool nb::OpenGl::OpenGlShader::isCompiled() const noexcept
{
    GLint isCompiled = 0;
    auto currentShader = shaders.back();

    glGetShaderiv(currentShader, GL_COMPILE_STATUS, &isCompiled);

    if (isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(currentShader, GL_INFO_LOG_LENGTH, &maxLength);

        GLint shader;
        glGetShaderiv(currentShader, GL_SHADER_TYPE, &shader);
        Debug::debug(shader);

        // The maxLength includes the NULL character
        std::string errorLog;
        glGetShaderInfoLog(currentShader, maxLength, &maxLength, &errorLog[0]);

        //
        // TODO 2: add message box about error
        Debug::debug(errorLog);

        glDeleteShader(currentShader); 
        return false;
    }

    return true;
}

nb::OpenGl::OpenGlShader::ShaderType nb::OpenGl::OpenGlShader::getShaderType(const std::filesystem::path &path) const noexcept
{
    auto extention = path.extension();

    if (extention == ".vs")
        return ShaderType::VERTEX;
    else if (extention == ".fs")
        return ShaderType::FRAGMENT;
    else if (extention == ".gs")
        return ShaderType::GEOMETRY;
    else if (extention == ".tcs")
        return ShaderType::TESSELLATION_CONTROLL;
    else if (extention == ".tes")
        return ShaderType::TESSELLATION_EVALUATION;
    else if (extention == ".cs")
        return ShaderType::COMPUTE;
    else
        return ShaderType::UNKNOWN;
}

std::string nb::OpenGl::OpenGlShader::loadFromFile(const std::filesystem::path &path) noexcept
{
    std::ifstream file;
    file.open(path, std::ios::in);
    if (!file.is_open())
    {
        nb::Fatal::exit(L"Cannot open shader");
        return nullptr;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return std::string(buffer.str());
}
