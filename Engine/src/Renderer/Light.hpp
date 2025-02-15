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
            virtual void applyUniforms(Ref<Renderer::Shader>& shader) = 0;
        };

        class Light : public IShadeble
        {
        public:

            static constexpr auto AMBIENT_UNIFORM_NAME = "La";
            static constexpr auto DIFFUSE_UNIFORM_NAME = "Ld";
            static constexpr auto SPECULAR_UNIFORM_NAME = "Ls";
            

            Light() = default;
            ~Light() = default;
            explicit Light(const Math::Vector3<float>& _ambient,
                            const Math::Vector3<float>& _diffuse,
                            const Math::Vector3<float>& _specular)
                : ambient(_ambient)
                , diffuse(_diffuse)
                , specular(_specular)
            {}

            virtual void applyUniforms(Ref<Renderer::Shader>& shader) override
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
            static constexpr auto CONST_COEFFICIENT_UNIFORM_NAME = "point_const_coof";
            static constexpr auto LINEAR_COEFFICIENT_UNIFORM_NAME = "point_linear_coof";
            static constexpr auto EXP_COEFFICIENT_UNIFORM_NAME = "point_exp_coof";

            PointLight() = default;
            ~PointLight() = default;
            explicit PointLight(const Math::Vector3<float>& _ambient,
                                const Math::Vector3<float>& _diffuse,
                                const Math::Vector3<float>& _specular,
                                const Math::Vector3<float>& _position,
                                float _constCoefficient,
                                float _linearCoefficient,
                                float _expCoefficient)
                : Light(_ambient, _diffuse, _specular)
                , position(_position)
                , constCoefficient(_constCoefficient)
                , linearCoefficient(_linearCoefficient)
                , expCoefficient(_expCoefficient)
            {}

            void applyUniforms(Ref<Renderer::Shader>& shader) override
            {
                Light::applyUniforms(shader);
                shader->setUniformVec3(POSITION_UNIFORM_NAME, position);
                shader->setUniformFloat(CONST_COEFFICIENT_UNIFORM_NAME, constCoefficient);
                shader->setUniformFloat(LINEAR_COEFFICIENT_UNIFORM_NAME, linearCoefficient);
                shader->setUniformFloat(EXP_COEFFICIENT_UNIFORM_NAME, expCoefficient);
            }

        private:
            float constCoefficient;
            float linearCoefficient;
            float expCoefficient;

            Math::Vector3<float> position;
        };


        class DirectionalLight : public Light
        {
        public:
            static constexpr auto DIRECTION_UNIFORM_NAME = "direction";

            DirectionalLight() = default;
            ~DirectionalLight() = default;

            explicit DirectionalLight(const Math::Vector3<float>& _ambient,
                                        const Math::Vector3<float>& _diffuse,
                                        const Math::Vector3<float>& _specular,
                                        const Math::Vector3<float>& _direction)
                : Light(_ambient, _diffuse, _specular)
                , direction(_direction)
            {}


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
        public:


        };
    };
};

#endif