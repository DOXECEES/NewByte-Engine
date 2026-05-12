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
            : IResource(path)
        {

        }

        void bind(Ref<Renderer::Shader> sh);

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
        std::string getShaderName() const { return toString(m_shader->getFilename()); }

        Ref<Renderer::Shader> getShader() {return m_shader;}
        void setShader(Ref<Renderer::Shader> shader)
        {
            m_shader = shader;
        }

        void updateMetaData() noexcept override
        {
            Loaders::Json j;

            if (m_shader)
            {
                j["shader"] = m_shader->getPath();
            }


            Loaders::Node node;


            for (auto& [name, prop] : m_properties)
            {
                std::visit(
                    [&](auto&& val)
                    {
                        using T = std::decay_t<decltype(val)>;

                        if constexpr (std::is_same_v<T, float> )
                        {
                            node[name] = val;
                        }
                        else if constexpr (std::is_same_v<T, nb::Color>)
                        {
                            auto rgba = val.toRgba();
                            node[name] = Loaders::Node::Array{rgba.r, rgba.g, rgba.b, rgba.alpha};
                        }
                        else if constexpr (std::is_same_v<T, Ref<TextureAsset>>)
                        {
                            if (val)
                            {
                                std::filesystem::path path = val->getPath();
                                path.replace_extension(".texture");
                                node[name] = path.string();
                            }
                            else
                            {
                                node[name] = nullptr;
                            }
                        }
                    },
                    prop.value
                );
            }

            j["properties"] = node;

            j.writeToFile(path);
        }



    private:
        Ref<Renderer::Shader> m_shader;
        std::unordered_map<std::string, MaterialProperty> m_properties;
        

        void loadFromPath(const std::filesystem::path& path);
    };

};

#endif