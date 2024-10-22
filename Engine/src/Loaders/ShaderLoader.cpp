#include "ShaderLoader.hpp"

//should return ref
// Ref<nb::Resource::ShaderSource> nb::Loaders::ShaderLoader::getRawShader(const std::filesystem::path &path) const noexcept
// {
//     if(!checkExtention(path))
//         return nullptr;

//     std::ifstream file;
//     file.open(path, std::ios::in);
//     if (!file.is_open())
//     {
//         nb::Fatal::exit(L"Cannot open shader");
//         return nullptr;
//     }

//     std::stringstream buffer;
//     buffer << file.rdbuf();

//     return createRef<nb::Resource::ShaderSource>(buffer.str());
// }

bool nb::Loaders::ShaderLoader::checkExtention(const std::filesystem::path &path) const noexcept
{
    return (std::find(VALID_EXTENTIONS.begin(), VALID_EXTENTIONS.end()
            , path.extension().string()) != VALID_EXTENTIONS.end());
}
