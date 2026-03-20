#ifndef SRC_RESOURCES_MATERIALASSET_HPP
#define SRC_RESOURCES_MATERIALASSET_HPP

#include <NbCore.hpp>

#include "Color.hpp"
#include "Core.hpp"
#include "Error/ErrorManager.hpp"
#include "Resources/IResource.hpp"
#include "Resources/TextureAsset.hpp"
#include "Renderer/Shader.hpp"
#include <Color.hpp>

#include <unordered_map>
#include <variant>

namespace nb::Resource
{
    struct MaterialProperty
    {
        std::variant<float, int, Color, Ref<TextureAsset>> value;
    };

    class MaterialAsset : public IResource
    {
    public:
        MaterialAsset(const std::filesystem::path& path)
            : m_path(path)
        {

        }

        void bind()
        {
            m_shader->use(); 

            int textureSlot = 0;
            for (auto& [name, prop] : m_properties) {
                if (std::holds_alternative<float>(prop.value)) {
                    m_shader->setUniformFloat(name, std::get<float>(prop.value));
                }
                else if (std::holds_alternative<Color>(prop.value)) {
                    m_shader->setUniformVec4(name, std::get<Color>(prop.value).asVec4());
                }
                else if (std::holds_alternative<Ref<TextureAsset>>(prop.value)) {
                    auto texAsset = std::get<Ref<TextureAsset>>(prop.value);
                    //texAsset->getInternalTexture()->bind(textureSlot);
                    //m_shader->setUniformInt(name, textureSlot);
                    textureSlot++;
                }
            }

        }


        void setProperty(
            const std::string& name,
            const Color& value
        )
        {
            m_properties[name] = MaterialProperty(value);
        }


        void setProperty(const std::string& name, float value)
        {
            m_properties[name] = MaterialProperty(value);

        }
        void setTexture(const std::string& name, Ref<TextureAsset> texture)
        {
            m_properties[name] = MaterialProperty(texture);
        }

        std::unordered_map<std::string, MaterialProperty>& getProperties() { return m_properties; }
        std::string getShaderName() const { return toString(m_shader->getId()); }

        Ref<Renderer::Shader> getShader() {return m_shader;}
        void setShader(Ref<Renderer::Shader> shader)
        {
            m_shader = shader;
        }


    private:
        Ref<Renderer::Shader> m_shader;
        std::unordered_map<std::string, MaterialProperty> m_properties;
        
        std::filesystem::path m_path;

        void loadFromPath(const std::filesystem::path& path);
    };

};

#endif