#ifndef SRC_LOADERS_SHADERLOADER_HPP
#define SRC_LOADERS_SHADERLOADER_HPP

#include <array>
#include <filesystem>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "../Fatal.hpp"
#include "../Resources/ShaderSource.hpp"

// future pch
#include "../Core.hpp"

namespace nb
{
    namespace Loaders
    {
        class ShaderLoader
        {
            constexpr static std::array<const char *, 6> VALID_EXTENTIONS = {".vs", ".fs", "gs", "tcs", "tes", "cs"};

        public:
            ShaderLoader() = default;
            ShaderLoader(const ShaderLoader&) = default;
            ShaderLoader(ShaderLoader&&) = default;
            ShaderLoader& operator=(const ShaderLoader&) = default;
            ShaderLoader& operator=(ShaderLoader&&) = default;
            ~ShaderLoader() = default;

            Ref<Resource::ShaderSource> getRawShader(const std::filesystem::path &path) const noexcept;

        private:
            bool checkExtention(const std::filesystem::path &path) const noexcept;

        };
    };
};

#endif