#include "EngineSettings.hpp"

namespace nb
{
    namespace Core
    {
        void EngineSettings::serialize()
        {
            
        }

        void EngineSettings::deserialize()
        {
            nb::Loaders::Json json;
            json.readFromFile(PATH_TO_CONFIG);
            width = json["width"].get<int>();
            height = json["height"].get<int>();
            fov = json["fov"].get<float>();
            graphicsApi = static_cast<Core::GraphicsAPI>(json["graphics-api"].get<int>());
        }

        int EngineSettings::width  = DEFAULT_WIDTH;
        int EngineSettings::height = DEFAULT_HEIGHT;
        float EngineSettings::fov = DEFAULT_FOV;
        Core::GraphicsAPI EngineSettings::graphicsApi = DEFAULT_GRAPHICS_API;
    };
};