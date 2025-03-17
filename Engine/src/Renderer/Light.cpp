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
