#include "ResourceManager.hpp"

// "/assets/terrain/tree.png" - абсолютный
// "tree.png" - относительный

Ref<nb::Resource::IResource> nb::ResMan::ResourceManager::getResource(std::string_view resName) const noexcept
{
    std::string path = resName.data();
    
    if(isRelativePath(resName))
    {
        path = "/assets/" + path;
    }

    if(!isResourceLoaded(path))
    {
        /// return placeholder
        // if(!load(path))
        // {
        //     return Ref<nb::Resource::IResource>();
        // }
    }

    return pool.at(path);
}

void nb::ResMan::ResourceManager::load(const std::string& path) const noexcept
{
    //if(path.ends_with(".vs"))
}

bool nb::ResMan::ResourceManager::isAbsolutePath(
    std::string_view path) const noexcept {
    return (!path.empty() && path.at(0) == '/');
}

bool nb::ResMan::ResourceManager::isRelativePath(std::string_view path) const noexcept
{
    return false;
}

bool nb::ResMan::ResourceManager::isResourceLoaded(std::string_view path) const noexcept
{
    return false;
}

// bool nb::ResMan::ResourceManager::isAbsolutePath(std::string_view path) const noexcept
// {
//     return !isAbsolutePath(path);
// }
