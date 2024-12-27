#ifndef SRC_RENDERER_LIGHT_HPP
#define SRC_RENDERER_LIGHT_HPP

#include "../Math/Vector3.hpp"

namespace nb
{
    namespace Renderer
    {
        class Light
        {

        public:

            Light() = default;
            ~Light() = default;
        
        private:
            


        };

        class PointLight : public Light
        {
        public:
            
            PointLight() = default;
            ~PointLight() = default;
        

        private:

            Math::Vector3<float> position;
            Math::Vector3<float> intensity;
        
        };


        class DirectionalLight : public Light
        {
        public:
        
            DirectionalLight() = default;
            ~DirectionalLight() = default;
        
        private:
            
            Math::Vector3<float> direction;
        };

    };
};

#endif