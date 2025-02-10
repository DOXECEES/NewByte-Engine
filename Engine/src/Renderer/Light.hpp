#ifndef SRC_RENDERER_LIGHT_HPP
#define SRC_RENDERER_LIGHT_HPP

#include "../Math/Vector3.hpp"
#include "Shader.hpp"

namespace nb
{
    namespace Renderer
    {
        class IShadeble
        {
        public:
            virtual void applyUniforms() = 0;
        };

        class Light : public IShadeble
        {
        public:

            static constexpr auto AMBIENT_UNIFORM_NAME = "La";
            static constexpr auto DIFFUSE_UNIFORM_NAME = "Ld";
            static constexpr auto SPECULAR_UNIFORM_NAME = "Ls";

            Light() = default;
            ~Light() = default;

            void applyUniforms(Ref<Renderer::Shader>& shader) override
            {
                shader->setUniformVec3(AMBIENT_UNIFORM_NAME, ambient);
                shader->setUniformVec3(DIFFUSE_UNIFORM_NAME, diffuse);
                shader->setUniformVec3(SPECULAR_UNIFORM_NAME, specular);
            };

        private:
    
            Math::Vector3<float> ambient;
            Math::Vector3<float> diffuse;
            Math::Vector3<float> specular;

        };

        class PointLight : public Light
        {
        public:
        
            static constexpr auto POSITION_UNIFORM_NAME = "position";
            static constexpr auto INTENSITY_UNIFORM_NAME = "intensity";
            
            PointLight() = default;
            ~PointLight() = default;
        
            void applyUniforms(Ref<Renderer::Shader>& shader) override
            {
                Light::applyUniforms(shader);
                shader->setUniformVec3(POSITION_UNIFORM_NAME, position);
                shader->setUniformVec3(INTENSITY_UNIFORM_NAME, intensity);
            }

        private:

            Math::Vector3<float> position;
            Math::Vector3<float> intensity;
        
        };


        class DirectionalLight : public Light
        {
        public:
            static constexpr auto DIRECTION_UNIFORM_NAME = "direction";

            DirectionalLight() = default;
            ~DirectionalLight() = default;

            void applyUniforms(Ref<Renderer::Shader>& shader) override
            {
                Light::applyUniforms(shader);
                shader->setUniformVec3(DIRECTION_UNIFORM_NAME, direction);
            }
        
        private:
            
            Math::Vector3<float> direction;
        };

        class SpotLight
        {      
        };
    };
};

#endif