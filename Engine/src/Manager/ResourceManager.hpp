#ifndef SRC_MANAGER_RESOURCEMANAGER_HPP
#define SRC_MANAGER_RESOURCEMANAGER_HPP

#include <memory>
#include <string_view>
#include <array>
#include <unordered_map> 

#include "../Resources/IResource.hpp"

#include "../Loaders/ShaderLoader.hpp"

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
            
            Ref<nb::Resource::IResource> getResource(std::string_view resName) const noexcept;

        private:

            void load(const std::string& path) const noexcept;
            void unload() noexcept;

            bool isAbsolutePath(std::string_view path) const noexcept;
            bool isRelativePath(std::string_view path) const noexcept;

            bool isResourceLoaded(std::string_view path) const noexcept;

            ResourceManager();
            ~ResourceManager();
        
        private:
            std::unordered_map<std::string, Ref<nb::Resource::IResource>> pool;
            //std::unordered_map<std::string, Loader>

        };
    };
};

#endif