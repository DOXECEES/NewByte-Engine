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


namespace nb::Renderer
{
    class IRenderAPI;
};

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
            , IResource(path)
        {

        }

        void bind(nb::Renderer::IRenderAPI* renderApi);

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