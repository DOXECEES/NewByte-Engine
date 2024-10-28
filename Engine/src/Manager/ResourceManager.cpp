#include "ResourceManager.hpp"

// "/assets/terrain/tree.png" - абсолютный
// "tree.png" - относительный

nb::ResMan::ResourceManager *nb::ResMan::ResourceManager::getInstance() noexcept
{
    static ResourceManager rm;
    return &rm;
}

void nb::ResMan::ResourceManager::load(const std::string& path) const noexcept
{
    //if(path.ends_with(".vs"))
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
    return false;
}

// bool nb::ResMan::ResourceManager::isAbsolutePath(std::string_view path) const noexcept
// {
//     return !isAbsolutePath(path);
// }
