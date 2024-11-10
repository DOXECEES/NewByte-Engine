#ifndef SRC_CORE_ENGINESETTINGS_HPP
#define SRC_CORE_ENGINESETTINGS_HPP

#include <filesystem>

#include "../Loaders/JSON/Json.hpp"

#include "../Core.hpp"

namespace nb
{
    namespace Core
    {
        class EngineSettings
        {
            /**
            * @todo Создать модуль файловых путей
            */ 
            inline static const std::filesystem::path PATH_TO_CONFIG = "config.json";

            static constexpr int DEFAULT_WIDTH = 640;
            static constexpr int DEFAULT_HEIGHT = 480;
            static constexpr Core::GraphicsAPI DEFAULT_GRAPHICS_API = Core::GraphicsAPI::OPENGL;


        public:
            static void serialize();
            static void deserialize();

            inline static int getWidth() noexcept { return width; };
            inline static int getHeight() noexcept { return height; };
            inline static Core::GraphicsAPI getGraphicsAPI() noexcept { return graphicsApi; };

        private:
        
            static int width;
            static int height;
            static Core::GraphicsAPI graphicsApi;
        };
    };
};

#endif
