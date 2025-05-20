#ifndef SRC_MANAGER_RESOURCEMANAGER_HPP
#define SRC_MANAGER_RESOURCEMANAGER_HPP

#include <memory>
#include <string_view>
#include <array>
#include <unordered_map>
#include <typeinfo>


#include "../Resources/IResource.hpp"
#include "../Loaders/Factory/IFactoryLoader.hpp"
#include "../Loaders/Factory/ShaderFactory.hpp"
#include "../Loaders/Factory/ObjFactory.hpp"

#include "../Loaders/ShaderLoader.hpp"
#include "../Loaders/JSON/Json.hpp"
#include "../Loaders/ILoader.hpp"

#include "../Core.hpp"

// shader

namespace nb
{
    namespace ResMan
    {
        class ResourceManager
        {
        public:
            using ResourcePool = std::unordered_map<std::string, Ref<nb::Resource::IResource>>; 

            static ResourceManager *getInstance() noexcept;

            template <typename T>
            Ref<T> getResource(std::string_view resourcePath) noexcept
            {
                std::string path(resourcePath);
                std::string_view extension = extractExtension(path);

                auto loaderIt = loaders.find(extension.data());
                if (loaderIt == loaders.end())
                {
                    Debug::debug("Unsupported file format: " + std::string(extension));
                    abort();
                }

                std::type_index type = loaderIt->second->getResourceType();

                if (!isResourceLoaded(path))
                {
                    load(path);
                }

                return std::dynamic_pointer_cast<T>(pool.at(type).at(path));
            }

            template <typename T>
            const ResourcePool& getAllResources() noexcept
            {
                std::type_index type = std::type_index(typeid(T));
                if(pool.find(type) == pool.end())
                {
                    Debug::debug("Resources of type: " + std::string(type.name()) + " not found in resource manager.");
                    abort();
                }
                return pool.at(type);
            }

            void registerLoader(std::string_view extention, Ref<nb::Loaders::Factory::IFactoryLoader> loader) noexcept;
            void createConcretePoolIfNotExists(std::type_index resourceType) noexcept;

        private:
            std::string_view extractExtension(std::string_view path) const noexcept;

            void load(const std::filesystem::path &path) noexcept;
            void unload() noexcept;

            bool isAbsolutePath(std::string_view path) const noexcept;
            bool isRelativePath(std::string_view path) const noexcept;

            bool isResourceLoaded(std::string_view path) const noexcept;

            ResourceManager();
            ~ResourceManager() = default;

        private:
            std::unordered_map<std::type_index, ResourcePool> pool;

            std::unordered_map<std::string, Ref<nb::Loaders::Factory::IFactoryLoader>> loaders;
        };
    };
};

#endif