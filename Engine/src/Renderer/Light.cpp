// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "Light.hpp"

namespace nb
{
    namespace Renderer
    {

        //Utils::Indexator Light::indexator = Utils::Indexator(0, 32);

        Utils::Indexator PointLight::indexator          = Utils::Indexator(0, 32);
        Utils::Indexator DirectionalLight::indexator    = Utils::Indexator(0, 32);
        Utils::Indexator SpotLight::indexator           = Utils::Indexator(0, 32);

    };

};
