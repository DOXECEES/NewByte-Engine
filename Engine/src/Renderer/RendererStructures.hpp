#ifndef SRC_RENDERER_RENDERERSTRUCTURES_HPP
#define SRC_RENDERER_RENDERERSTRUCTURES_HPP

#include "../Core.hpp"

#include "../Math/Vector2.hpp"
#include "../Math/Vector3.hpp"

#include "../Math/Matrix/Matrix.hpp"

#include "Shader.hpp"


#include <vector>
#include <map>
#include <string>

namespace nb
{
    namespace Renderer
    {

        struct Vertex
        {
            Vertex() = default;

            Vertex(const Math::Vector3<float> &pos, const Math::Vector3<float> &norm = 0, const Math::Vector3<float>& color = 0, const Math::Vector2<float> &tex = 0)
                : position(pos), normal(norm), color(color), textureCoodinates(tex)
            {
            }
            Math::Vector3<float> position;
            Math::Vector3<float> normal;
            Math::Vector3<float> color;
            Math::Vector2<float> textureCoodinates;

            bool operator==(const Vertex &other) const
            {
                return position == other.position &&
                       normal == other.normal &&
                       color == other.color &&
                       textureCoodinates == other.textureCoodinates;
            }
        };

        struct Material
        {
            Math::Vector3<float> ambient;
            Math::Vector3<float> diffuse;
            Math::Vector3<float> specular;
            float shininess;

            void lerp(const Material& target, float alpha)
            {
                ambient.x += (target.ambient.x - ambient.x) * alpha;
                ambient.y += (target.ambient.y - ambient.y) * alpha;
                ambient.z += (target.ambient.z - ambient.z) * alpha;
                diffuse.x += (target.diffuse.x - diffuse.x) * alpha;
                diffuse.y += (target.diffuse.y - diffuse.y) * alpha;
                diffuse.z += (target.diffuse.z - diffuse.z) * alpha;
                specular.x += (target.specular.x - specular.x) * alpha;
                specular.y += (target.specular.y - specular.y) * alpha;
                specular.z += (target.specular.z - specular.z) * alpha;
                shininess += (target.shininess - shininess) * alpha;
            }
        };

        struct MaterialNode
        {
            Ref<Shader> shader;
            std::map<std::string, float> floatUniforms;
            std::map<std::string, Math::Vector3<float>> vec3Uniforms;
            std::map<std::string, Math::Vector4<float>> vec4Uniforms;
            std::map<std::string, Math::Mat4<float>> mat4Uniforms;

            void applyMaterial() 
            {
                for(const auto&i : floatUniforms)
                {
                    shader->setUniformFloat(i.first, i.second);
                }

                for(const auto&i : vec3Uniforms)
                {
                    shader->setUniformVec3(i.first, i.second);
                }

                for(const auto&i : vec4Uniforms)
                {
                    shader->setUniformVec4(i.first, i.second);
                }

                for(const auto&i : mat4Uniforms)
                {
                    shader->setUniformMat4(i.first, i.second);
                }
            }
        };
    };
};

#endif
