#include "ResourceManager.hpp"

namespace nb
{
    namespace ResMan
    {
        ResourceManager::ResourceManager()
        {
            regisrterLoader(".json", createRef<nb::Loaders::JsonFactory>());
            regisrterLoader(".nbsd", createRef<nb::Loaders::JsonFactory>());
            regisrterLoader(".shader", createRef<nb::Loaders::Factory::ShaderFactory>());
            regisrterLoader(".obj", createRef<nb::Loaders::Factory::ObjFactory>());
        }

        ResourceManager *nb::ResMan::ResourceManager::getInstance() noexcept
        {
            static ResourceManager rm;
            return &rm;
        }

        void ResourceManager::regisrterLoader(std::string_view extention, Ref<nb::Loaders::Factory::IFactoryLoader> loader) noexcept
        {
            loaders[extention.data()] = loader;
            createConcretePoolIfNotExists(loader->getResourceType());            
        }

        void ResourceManager::createConcretePoolIfNotExists(std::type_index resourceType) noexcept
        {
            if(pool.find(resourceType) == pool.end())
            {
                pool[resourceType] = ResourcePool();
            }
        }
        std::string_view ResourceManager::extractExtention(std::string_view path) const noexcept
        {
            return path.substr(path.find_last_of("."));
        }

        std::string ResourceManager::extractExtention(std::string path) const noexcept
        {
            return path.substr(path.find_last_of("."));
        }

        void ResourceManager::load(const std::filesystem::path &path) noexcept
        {
            std::string extention = path.extension().string();
            if(loaders.find(extention.data()) == loaders.end()) 
            {
                Debug::debug("Loader not found");
                abort();
            }

            std::type_index type = loaders.at(extention.data())->getResourceType();   
            pool[type][path.string()] = loaders.at(extention.data())->create(path);
        }

        bool ResourceManager::isAbsolutePath( std::string_view path) const noexcept
        {
            return (!path.empty() && path.at(0) == '/');
        }

        bool ResourceManager::isRelativePath(std::string_view path) const noexcept
        {
            return !isAbsolutePath(path);
        }

        bool ResourceManager::isResourceLoaded(std::string_view path) const noexcept
        {
            std::string_view extention = extractExtention(path);
            if(loaders.find(extention.data()) == loaders.end()) 
            {
                Debug::debug("Loader not found");
                abort();
            }

            std::type_index type = loaders.at(extention.data())->getResourceType();   
            return pool.at(type).contains(path.data());        
        }

    };
};


