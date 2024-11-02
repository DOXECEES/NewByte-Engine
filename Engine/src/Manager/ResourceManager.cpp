#include "ResourceManager.hpp"

// "/assets/terrain/tree.png" - абсолютный
// "tree.png" - относительный

nb::ResMan::ResourceManager::ResourceManager()
{
    // make pool of loaders
    loaders[".json"] = createRef<nb::Loaders::JsonFactory>();
    loaders[".nbsd"] = loaders[".json"];
    loaders[".shader"] = createRef<nb::Loaders::Factory::ShaderFactory>();
}


nb::ResMan::ResourceManager *nb::ResMan::ResourceManager::getInstance() noexcept
{
    static ResourceManager rm;
    return &rm;
}

void nb::ResMan::ResourceManager::load(const std::filesystem::path& path) noexcept
{
    auto j = loaders.at(path.extension().string())->create(path);
    pool[path.string()] = j;
}

bool nb::ResMan::ResourceManager::isAbsolutePath( std::string_view path) const noexcept {
    return (!path.empty() && path.at(0) == '/');
}

bool nb::ResMan::ResourceManager::isRelativePath(std::string_view path) const noexcept
{
    return !isAbsolutePath(path);
}

bool nb::ResMan::ResourceManager::isResourceLoaded(std::string_view path) const noexcept
{
    return pool.contains(path.data());
}



// bool nb::ResMan::ResourceManager::isAbsolutePath(std::string_view path) const noexcept
// {
//     return !isAbsolutePath(path);
// }
