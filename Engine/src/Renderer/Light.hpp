#ifndef SRC_RENDERER_LIGHT_HPP
#define SRC_RENDERER_LIGHT_HPP

#include "../Math/Vector3.hpp"
#include "../Utils/Indexator.hpp"

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
            static constexpr std::string_view GLOABL_LIGHTS_STORE_UNIFORM_NAME = "light";
            static constexpr std::string_view AMBIENT_UNIFORM_NAME  = "La";
            static constexpr std::string_view DIFFUSE_UNIFORM_NAME  = "Ld";
            static constexpr std::string_view SPECULAR_UNIFORM_NAME = "Ls";

            Light() = default;
            explicit Light(const Math::Vector3<float>& _ambient,
                            const Math::Vector3<float>& _diffuse,
                            const Math::Vector3<float>& _specular)
                : ambient(_ambient)
                , diffuse(_diffuse)
                , specular(_specular)
                , id(indexator.index())
            {}

            virtual void applyUniforms(Ref<Renderer::Shader>& shader) override
            {
                const std::string ambientUniformName  = makeUniformName(GLOABL_LIGHTS_STORE_UNIFORM_NAME, id, AMBIENT_UNIFORM_NAME);
                const std::string diffuseUniformName  = makeUniformName(GLOABL_LIGHTS_STORE_UNIFORM_NAME, id, DIFFUSE_UNIFORM_NAME);
                const std::string specularUniformName = makeUniformName(GLOABL_LIGHTS_STORE_UNIFORM_NAME, id, SPECULAR_UNIFORM_NAME);
                
                shader->setUniformVec3(ambientUniformName, ambient);
                shader->setUniformVec3(diffuseUniformName, diffuse);
                shader->setUniformVec3(specularUniformName, specular);
            };

            int getId() const noexcept
            {
                return id;
            }

            ~Light()
            {
                indexator.freeIndex(id);
            }
        
        protected:

            std::string makeUniformName(std::string_view base, int id, std::string_view property)
            {
                std::string result;
                result.reserve(base.size() + property.size() + COUNT_OF_CHARS_TO_WRITE_INT);
                result.append(base).append("[").append(std::to_string(id)).append("].").append(property);
                return result;
            }
    
            Math::Vector3<float> ambient;
            Math::Vector3<float> diffuse;
            Math::Vector3<float> specular;
            int id;

        private:
            static constexpr uint8_t COUNT_OF_CHARS_TO_WRITE_INT = 10; 

            static Utils::Indexator indexator;
        
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

                const std::string positionUniformName = makeUniformName(GLOABL_LIGHTS_STORE_UNIFORM_NAME, id, POSITION_UNIFORM_NAME);
                const std::string constCoefficientUniformName = makeUniformName(GLOABL_LIGHTS_STORE_UNIFORM_NAME, id, CONST_COEFFICIENT_UNIFORM_NAME);
                const std::string linearCoefficientUniformName = makeUniformName(GLOABL_LIGHTS_STORE_UNIFORM_NAME, id, LINEAR_COEFFICIENT_UNIFORM_NAME);
                const std::string expCoefficientUniformName = makeUniformName(GLOABL_LIGHTS_STORE_UNIFORM_NAME, id, EXP_COEFFICIENT_UNIFORM_NAME);

                shader->setUniformVec3(positionUniformName, position);
                shader->setUniformFloat(constCoefficientUniformName, constCoefficient);
                shader->setUniformFloat(linearCoefficientUniformName, linearCoefficient);
                shader->setUniformFloat(expCoefficientUniformName, expCoefficient);
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

                const std::string directionUniformName = makeUniformName(GLOABL_LIGHTS_STORE_UNIFORM_NAME, id, DIRECTION_UNIFORM_NAME);
                shader->setUniformVec3(directionUniformName, direction);
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