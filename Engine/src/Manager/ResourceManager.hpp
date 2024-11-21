#ifndef SRC_MANAGER_RESOURCEMANAGER_HPP
#define SRC_MANAGER_RESOURCEMANAGER_HPP

#include <memory>
#include <string_view>
#include <array>
#include <unordered_map>

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
            enum class ResourceType
            {
                NONE = -1,
                TEXTURE,
                SHADER,
                COUNT,
            };

            static ResourceManager *getInstance() noexcept;

            // если путь не начинается с /, то будет использована стандартная директория ассетов
            template <typename T>
            Ref<T> getResource(std::string_view resName) noexcept
            {
                std::string path = resName.data();

                //if (isRelativePath(resName))
                //{
                //    path = "/assets/" + path;
                //}

                if (!isResourceLoaded(path))
                {
                    /// return placeholder
                    load(path);
                }

                return std::dynamic_pointer_cast<T>(pool.at(path));
            }

        private:
            void load(const std::filesystem::path &path) noexcept;
            void unload() noexcept;

            bool isAbsolutePath(std::string_view path) const noexcept;
            bool isRelativePath(std::string_view path) const noexcept;

            bool isResourceLoaded(std::string_view path) const noexcept;

            ResourceManager();
            ~ResourceManager() = default;

        private:
            std::unordered_map<std::string, Ref<nb::Resource::IResource>> pool;
            std::unordered_map<std::string, Ref<nb::Loaders::Factory::IFactoryLoader>> loaders;
        };
    };
};

#endif