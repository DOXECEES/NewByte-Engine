#ifndef SRC_RENDERER_LIGHT_HPP
#define SRC_RENDERER_LIGHT_HPP

#include "../Math/Vector3.hpp"
#include "../Utils/Indexator.hpp"

#include "Shader.hpp"

namespace nb
{
    namespace Renderer
    {
        class IShadable
        {
        public:
            virtual void applyUniforms(Ref<Shader>& shader) = 0;
        };

        // все классы наследники должны определять в себе стстический индексатор
        // и высвобождать индекс в деструкторе
        class Light : public IShadable
        {
        public:
            static constexpr std::string_view GLOBAL_LIGHTS_STORE_UNIFORM_NAME          = "light";
            static constexpr std::string_view GLOBAL_POINT_LIGHTS_STORE_UNIFORM_NAME    = "lightPoint";

            static constexpr std::string_view POSITION_UNIFORM_NAME                     = "position";
            static constexpr std::string_view AMBIENT_UNIFORM_NAME                      = "La";
            static constexpr std::string_view DIFFUSE_UNIFORM_NAME                      = "Ld";
            static constexpr std::string_view SPECULAR_UNIFORM_NAME                     = "Ls";
        
            Light() = default;
            virtual ~Light() = default;
            explicit Light(const Math::Vector3<float>& _ambient,
                            const Math::Vector3<float>& _diffuse,
                            const Math::Vector3<float>& _specular,
                            const Math::Vector3<float>& _position = 0)
                : ambient(_ambient)
                , diffuse(_diffuse)
                , specular(_specular)
                , position(_position)
            {}

            virtual void applyUniforms(Ref<Shader>& shader) = 0;

            void applyBaseUniforms(
                Ref<Shader>& shader,
                std::string_view base
            )
            {
                const std::string ambientUniformName  = makeUniformName(base, id, AMBIENT_UNIFORM_NAME);
                const std::string diffuseUniformName  = makeUniformName(base, id, DIFFUSE_UNIFORM_NAME);
                const std::string specularUniformName = makeUniformName(base, id, SPECULAR_UNIFORM_NAME);
                
                shader->setUniformVec3(ambientUniformName, ambient);
                shader->setUniformVec3(diffuseUniformName, diffuse);
                shader->setUniformVec3(specularUniformName, specular);
            }

            int getId() const noexcept
            {
                return id;
            }

            const Math::Vector3<float>& getPosition() const noexcept
            {
                return position;
            }

            void setPosition(const Math::Vector3<float>& _position) noexcept
            {
                position = _position;
            }

            const Math::Vector3<float>& getAmbient() const noexcept
            {
                return ambient;
            }

            const Math::Vector3<float>& getDiffuse() const noexcept
            {
                return diffuse;
            }
            
        protected:

            std::string makeUniformName(
                std::string_view base,
                int uniformId,
                std::string_view property
            ) const
            {
                char idBuffer[COUNT_OF_CHARS_TO_WRITE_INT + 1]; // '\0'
                int len = std::snprintf(idBuffer, sizeof(idBuffer), "%d", uniformId);

                std::string result;
                result.reserve(base.size() + property.size() + len + 3); // +3 на "[]."
                result.append(base).append("[").append(idBuffer, len).append("].").append(property);
                return result;

            }

            Math::Vector3<float>    position;

    
            Math::Vector3<float>    ambient;
            Math::Vector3<float>    diffuse;
            Math::Vector3<float>    specular;

            int                     id          = 0;

        private:
            static constexpr uint8_t COUNT_OF_CHARS_TO_WRITE_INT = 11; // включая минус

        
        };

        class PointLight : public Light
        {
        public:
        
            //static constexpr auto POSITION_UNIFORM_NAME = "position";
            static constexpr auto INTENSITY_UNIFORM_NAME            = "intensity";
            static constexpr auto CONST_COEFFICIENT_UNIFORM_NAME    = "point_const_coof";
            static constexpr auto LINEAR_COEFFICIENT_UNIFORM_NAME   = "point_linear_coof";
            static constexpr auto EXP_COEFFICIENT_UNIFORM_NAME      = "point_exp_coof";


            PointLight() = default;
            
            ~PointLight() override
            {
                indexator.freeIndex(id);
            }

            explicit PointLight(const Math::Vector3<float>& _ambient,
                                const Math::Vector3<float>& _diffuse,
                                const Math::Vector3<float>& _specular,
                                const Math::Vector3<float>& _position,
                                float _constCoefficient,
                                float _linearCoefficient,
                                float _expCoefficient,
                                float _intensity)
                : Light(_ambient, _diffuse, _specular,_position)
                , constCoefficient(_constCoefficient)
                , linearCoefficient(_linearCoefficient)
                , expCoefficient(_expCoefficient)
                , intensity(_intensity)
            {
                id = indexator.index();
            }

            void applyUniforms(Ref<Shader>& shader) override
            {
                Light::applyBaseUniforms(shader, GLOBAL_POINT_LIGHTS_STORE_UNIFORM_NAME);

                const std::string positionUniformName = makeUniformName(GLOBAL_POINT_LIGHTS_STORE_UNIFORM_NAME, id, POSITION_UNIFORM_NAME);
                const std::string constCoefficientUniformName = makeUniformName(GLOBAL_POINT_LIGHTS_STORE_UNIFORM_NAME, id, CONST_COEFFICIENT_UNIFORM_NAME);
                const std::string linearCoefficientUniformName = makeUniformName(GLOBAL_POINT_LIGHTS_STORE_UNIFORM_NAME, id, LINEAR_COEFFICIENT_UNIFORM_NAME);
                const std::string expCoefficientUniformName = makeUniformName(GLOBAL_POINT_LIGHTS_STORE_UNIFORM_NAME, id, EXP_COEFFICIENT_UNIFORM_NAME);
                const std::string intensityUniformName = makeUniformName(GLOBAL_POINT_LIGHTS_STORE_UNIFORM_NAME, id, INTENSITY_UNIFORM_NAME);
                
                shader->setUniformVec3(positionUniformName, position);
                shader->setUniformFloat(constCoefficientUniformName, constCoefficient);
                shader->setUniformFloat(linearCoefficientUniformName, linearCoefficient);
                shader->setUniformFloat(expCoefficientUniformName, expCoefficient);
                shader->setUniformFloat(intensityUniformName, intensity);
            }

            static int getCountOfPointLights() noexcept
            {
                return indexator.next() - 1;
            }

        private:
            float                   constCoefficient;
            float                   linearCoefficient;
            float                   expCoefficient;

            float                   intensity;

            static Utils::Indexator indexator;
        };


        class DirectionalLight : public Light
        {
        public:
            static constexpr auto DIRECTION_UNIFORM_NAME = "direction";

            DirectionalLight() = default;
            ~DirectionalLight() override
            {
                indexator.freeIndex(id);
            }

            explicit DirectionalLight(const Math::Vector3<float>& _ambient,
                                        const Math::Vector3<float>& _diffuse,
                                        const Math::Vector3<float>& _specular,
                                        const Math::Vector3<float>& _direction)
                : Light(_ambient, _diffuse, _specular)
                , direction(_direction)
            {
                id = indexator.index();
            }

            void applyUniforms(Ref<Shader>& shader) override
            {
                Light::applyBaseUniforms(shader, GLOBAL_LIGHTS_STORE_UNIFORM_NAME);

                const std::string directionUniformName = makeUniformName(GLOBAL_LIGHTS_STORE_UNIFORM_NAME, id, DIRECTION_UNIFORM_NAME);
                shader->setUniformVec3(directionUniformName, direction);
            }

            static int getCountOfDirectionalLights() noexcept
            {
                return indexator.next() - 1;
            }
        
        private:
            
            Math::Vector3<float>    direction;

            static Utils::Indexator indexator;

        };

        class SpotLight : public Light
        {      
        public:
            
            ~SpotLight() override
            {
                indexator.freeIndex(id);
            }


            static int getCountOfSpotLights() noexcept
            {
                return indexator.next() - 1;
            }

        private:
            static Utils::Indexator indexator;

        };
    };
};

#endif