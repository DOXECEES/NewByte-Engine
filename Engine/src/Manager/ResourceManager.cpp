#include "ResourceManager.hpp"

namespace nb
{
    namespace ResMan
    {
        ResourceManager::ResourceManager()
        {
            registerLoader(".json", createRef<nb::Loaders::JsonFactory>());
            registerLoader(".nbsd", createRef<nb::Loaders::JsonFactory>());
            registerLoader(".shader", createRef<nb::Loaders::Factory::ShaderFactory>());
            registerLoader(".obj", createRef<nb::Loaders::Factory::ObjFactory>());
        }

        ResourceManager *nb::ResMan::ResourceManager::getInstance() noexcept
        {
            static ResourceManager rm;
            return &rm;
        }

        void ResourceManager::registerLoader(std::string_view extention, Ref<nb::Loaders::Factory::IFactoryLoader> loader) noexcept
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
        std::string_view ResourceManager::extractExtension(std::string_view path) const noexcept 
        {
            size_t dotPos = path.find_last_of('.');
            if (dotPos == std::string_view::npos) return {};
            return path.substr(dotPos);
        }

        void ResourceManager::load(const std::filesystem::path &path) noexcept
        {
            std::string extension = path.extension().string();
            auto loaderIterator = loaders.find(extension);
            if (loaderIterator == loaders.end())
            {
                Debug::debug("Loader not found for extension: " + extension);
                abort();
            }

            std::type_index type = loaderIterator->second->getResourceType();   
            pool[type][path.string()] = loaderIterator->second->create(path);
        }

        void ResourceManager::unload() noexcept
        {
            for(auto& concretePool : pool)
            {
                concretePool.second.clear();
            }
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
            std::string_view extension = extractExtension(path);

            auto loaderIterator = loaders.find(extension.data());
            if (loaderIterator == loaders.end())
            {
                Debug::debug("Loader not found for extension: " + std::string(extension));
                abort();
            }

            std::type_index type = loaderIterator->second->getResourceType();
            const auto& concretePool = pool.at(type);

            return concretePool.contains(path.data());
        }


    };
};


