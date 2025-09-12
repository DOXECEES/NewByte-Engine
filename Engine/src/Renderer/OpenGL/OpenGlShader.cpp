// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "OpenGlShader.hpp"

nb::OpenGl::OpenGlShader::OpenGlShader(const std::filesystem::path &pathToShader) noexcept
{
    pathsToShaderSources.push_back(pathToShader);
    link(pathToShader);
    createProgram();
}

nb::OpenGl::OpenGlShader::OpenGlShader(const std::vector<std::filesystem::path> &vecOfShaders) noexcept
{
    pathsToShaderSources = vecOfShaders;
    for (const auto &shaderPath : vecOfShaders)
    {
        link(shaderPath);
    }

    createProgram();
}

nb::OpenGl::OpenGlShader::~OpenGlShader() noexcept
{
    for (auto &i : shaders)
    {
        glDeleteShader(i);
    }
    glDeleteProgram(program);
}

void nb::OpenGl::OpenGlShader::recompile() noexcept
{
    for (auto &i : shaders)
    {
        glDeleteShader(i);
    }
    glDeleteProgram(program);

    for (const auto &shaderPath : pathsToShaderSources)
    {
        link(shaderPath);
    }
    createProgram();
}

void nb::OpenGl::OpenGlShader::link(const std::filesystem::path &pathToShader) noexcept
{
    Debug::debug("Loading shader: " + pathToShader.string());
    if (auto shaderType = getShaderType(pathToShader); shaderType != ShaderType::UNKNOWN)
    {
        shaders.push_back(glCreateShader(static_cast<GLenum>(shaderType)));
        load(pathToShader);
    }
    else
    {
        assert(L"Shader type not supported");
    }
}

void nb::OpenGl::OpenGlShader::use() noexcept
{
    return glUseProgram(program);
}

void nb::OpenGl::OpenGlShader::setUniformFloat(std::string_view name, const float value) const noexcept
{
    GLint loc = glGetUniformLocation(program, name.data());
    glProgramUniform1f(program, loc, value);
}

void nb::OpenGl::OpenGlShader::setUniformInt(std::string_view name, const int value) const noexcept
{
    GLint loc = glGetUniformLocation(program, name.data());
    glProgramUniform1i(program, loc, value);
}

void nb::OpenGl::OpenGlShader::setUniformVec2(std::string_view name, const Math::Vector2<float> value) const noexcept
{
    GLint loc = glGetUniformLocation(program, name.data());
    glProgramUniform2f(program, loc, value.x, value.y);
}

void nb::OpenGl::OpenGlShader::setUniformVec3(std::string_view name, const Math::Vector3<float> value) const noexcept
{
    GLint loc = glGetUniformLocation(program, name.data());
    glProgramUniform3f(program, loc, value.x, value.y, value.z);
}

void nb::OpenGl::OpenGlShader::setUniformVec4(std::string_view name, const Math::Vector4<float> value) const noexcept
{
    GLint loc = glGetUniformLocation(program, name.data());
    glProgramUniform4f(program, loc, value.x, value.y, value.z, value.w);
}

void nb::OpenGl::OpenGlShader::setUniformMat2(std::string_view name, const Math::Mat2<float> &value) const noexcept
{
    GLint loc = glGetUniformLocation(program, name.data());
    glProgramUniformMatrix2fv(program, loc, 1, GL_FALSE, value.valuePtr());
}

void nb::OpenGl::OpenGlShader::setUniformMat3(std::string_view name, const Math::Mat3<float> &value) const noexcept
{
    GLint loc = glGetUniformLocation(program, name.data());
    glProgramUniformMatrix3fv(program, loc, 1, GL_FALSE, value.valuePtr());
}

void nb::OpenGl::OpenGlShader::setUniformMat4(std::string_view name, const Math::Mat4<float> &value) const noexcept
{
    GLint loc = glGetUniformLocation(program, name.data());
    glProgramUniformMatrix4fv(program, loc, 1, GL_FALSE, value.valuePtr());
}



void nb::OpenGl::OpenGlShader::createProgram() noexcept
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
        for (auto &i : shaders)
        {
            glDeleteShader(i);
        }
        Debug::debug(infoLog);
        assert(L"Cannot link shader");
    }

    for (const auto &i : shaders)
    {
        glDetachShader(program, i);
    }
}

void nb::OpenGl::OpenGlShader::load(const std::filesystem::path &pathToShader) noexcept
{
    auto s = loadFromFile(pathToShader);
    const char *shaderSource = s.c_str();
    glShaderSource(shaders.back(), 1, &shaderSource, NULL);
    glCompileShader(shaders.back());

    if (!isCompiled())
    {
        assert(L"Cannot compile shader");
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
        char *arr = new char[maxLength];
        // char *errorLog = new char[maxLength];
        glGetShaderInfoLog(currentShader, maxLength, &maxLength, arr);

        // TODO 2: add message box about error
        Debug::debug(arr);
        delete[] arr;

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
        assert(L"Cannot open shader");
        return nullptr;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    
    file.close();

    return buffer.str();
}
