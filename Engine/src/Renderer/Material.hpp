#ifndef SRC_RENDERER_MATERIAL_HPP
#define SRC_RENDERER_MATERIAL_HPP

#include "Core.hpp"

#include <map>
#include <string>

#include "Renderer/Shader.hpp"
#include "IRenderAPI.hpp"
#include "OpenGL/OpenGLTexture.hpp"

namespace nb::Renderer
{
    class MaterialNew
    {
    public:
        MaterialNew(Ref<Shader> shader)
            : shader(shader)
        {
            state.shader = shader;
            state.polygonMode = PolygonMode::FULL;
            state.isDepthTestEnable = true;
            state.isBlendEnable = true;
        }

        void setVector3(const std::string& name, const Math::Vector3<float>& value)
        {
            vectors3[name] = value;
        }

        void setFloat(const std::string& name, float value)
        {
            floats[name] = value;
        }

        void setInt(const std::string& name, int value)
        {
            ints[name] = value;
        }

        void setTexture(uint32 slot, Ref<OpenGl::OpenGlTexture> tex) 
        {
            textures[slot] = tex;
        }

        void setPolygonMode(PolygonMode mode) { state.polygonMode = mode; }

        PipelineHandle getPipeline(PipelineCache& cache) 
        {
            return cache.getOrCreate(state);
        }

        void apply(IRenderAPI* api) 
        {
            shader->use();

            for (auto& [name, val] : vectors3) shader->setUniformVec3(name, val);
            for (auto& [name, val] : floats)   shader->setUniformFloat(name, val);
            for (auto& [name, val] : ints) shader->setUniformInt(name, val);

            for (auto& [slot, tex] : textures) 
            {
                api->bindTexture(slot, tex->getId());
            }

        }

    private:
        Ref<Shader> shader;
        Pipeline state;

        std::map<std::string, Math::Vector2<float>> vectors2;
        std::map<std::string, Math::Vector3<float>> vectors3;
        std::map<std::string, Math::Vector4<float>> vectors4;
        std::map<std::string, float> floats;
        std::map<std::string, int> ints;
        std::map<uint32, Ref<OpenGl::OpenGlTexture>> textures;

    };


    class PBRMaterial : public MaterialNew
    {
    public:
        PBRMaterial(Ref<Shader> shader) 
            : MaterialNew(shader)
        {
            setFloat("u_Metallic", 0.0f);
            setFloat("u_Roughness", 0.5f);
            setFloat("u_AO", 1.0f);
            setVector3("u_Albedo", { 0.0f, 1.0f, 1.0f });
        }

        void setAlbedoMap(Ref<OpenGl::OpenGlTexture> tex) { setTexture(0, tex); }
        void setNormalMap(Ref<OpenGl::OpenGlTexture> tex) { setTexture(1, tex); }
        void setMetallicMap(Ref<OpenGl::OpenGlTexture> tex) { setTexture(2, tex); }
        void setRoughnessMap(Ref<OpenGl::OpenGlTexture> tex) { setTexture(3, tex); }
        void setAmbientOcclusionMap(Ref<OpenGl::OpenGlTexture> tex) { setTexture(4, tex); }
        
    };


};


#endif