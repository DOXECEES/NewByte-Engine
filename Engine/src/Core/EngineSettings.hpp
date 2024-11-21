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
            inline static const std::filesystem::path PATH_TO_CONFIG = "C:\\rep\\Hex\\NewByte-Engine\\build\\Engine\\Debug\\config.json";

            static constexpr int DEFAULT_WIDTH = 640;
            static constexpr int DEFAULT_HEIGHT = 480;
            static constexpr float DEFAULT_FOV = 45.0f;
            static constexpr Core::GraphicsAPI DEFAULT_GRAPHICS_API = Core::GraphicsAPI::OPENGL;


        public:
            static void serialize();
            static void deserialize();

            inline static int getWidth() noexcept { return width; };
            inline static int getHeight() noexcept { return height; };
            inline static float getFov() noexcept { return fov; };
            inline static float getAspectRatio() noexcept { return (static_cast<float>(width) / static_cast<float>(height)); };
            inline static Core::GraphicsAPI getGraphicsAPI() noexcept { return graphicsApi; };

            inline static void setWidth(const int newWidth) noexcept { width = newWidth; };
            inline static void setHeight(const int newHeight) noexcept { height = newHeight; };
            inline static void setFov(const float newFov) noexcept { fov = newFov; };

        private:
        
            static int width;
            static int height;
            static float fov;
            static Core::GraphicsAPI graphicsApi;
        };
    };
};

#endif
