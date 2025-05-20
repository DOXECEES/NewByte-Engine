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

            // если путь не начинается с /, то будет использована стандартная директория ассетов
            template <typename T>
            Ref<T> getResource(std::string_view resName) noexcept
            {
                std::string path = resName.data();

                std::string_view extention = extractExtention(resName);

                if (loaders.find(extention.data()) == loaders.end())
                {
                    Debug::debug("File format not supported");
                    abort();
                }

                std::type_index resourceType = loaders[extention.data()]->getResourceType();

                if (!isResourceLoaded(path))
                {
                    /// return placeholder
                    load(path);
                }


                return std::dynamic_pointer_cast<T>(pool.at(resourceType).at(path));
            }

            void regisrterLoader(std::string_view extention, Ref<nb::Loaders::Factory::IFactoryLoader> loader) noexcept;
            void createConcretePoolIfNotExists(std::type_index resourceType) noexcept;

        private:
            std::string_view extractExtention(std::string_view path) const noexcept;
            std::string extractExtention(std::string path) const noexcept;

            void load(const std::filesystem::path &path) noexcept;
            void unload() noexcept;

            bool isAbsolutePath(std::string_view path) const noexcept;
            bool isRelativePath(std::string_view path) const noexcept;

            bool isResourceLoaded(std::string_view path) const noexcept;

            ResourceManager();
            ~ResourceManager() = default;

        private:
            std::unordered_map<std::type_index, ResourcePool> pool;

            // pool -> concrete pools
            // concrete pools -> resources
            
            std::unordered_map<std::string, Ref<nb::Loaders::Factory::IFactoryLoader>> loaders;
        };
    };
};

#endif