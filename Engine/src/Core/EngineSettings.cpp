// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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
            mouseSensivity = json["mouse-sensivity"].get<float>();
            graphicsApi = static_cast<Core::GraphicsAPI>(json["graphics-api"].get<int>());
        }

  
    };
};