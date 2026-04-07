#include "MaterialLoader.hpp"

#include "JSON/Json.hpp"
#include "Manager/ResourceManager.hpp"

namespace nb::Loaders
{
    Ref<Resource::MaterialAsset>
    MaterialLoader::loadAsset(const std::filesystem::path& path) noexcept
    {
        try
        {
            nb::Loaders::Json json;
            json.readFromFile(path);
            

            auto material = createRef<Resource::MaterialAsset>(path);

           
           std::string shaderPath = json["shader"].get<std::string>();
           auto shader = nb::ResMan::ResourceManager::getInstance()->getResource<nb::Renderer::Shader>(
                        shaderPath
                    );




            material->setShader(shader);
            
            {
                auto& propsNode = json["properties"];
                if (propsNode.isObject())
                {
                    for (auto const& [name, node] : propsNode.get<Node::Map>())
                    {
                        if (node.isValue())
                        {
                            if (node.isString())
                            {
                                std::string texPath = node.get<std::string>();
                                auto texture =
                                    nb::ResMan::ResourceManager::getInstance()
                                        ->getResource<nb::Resource::TextureAsset>(texPath);

                                material->setTexture(name, texture);
                            }
                            else
                            {
                                material->setProperty(name, node.get<float>());
                            }
                        }
                        else if (node.isArray())
                        {
                            auto colorData = node.getArray<float>();
                            if (colorData.size() >= 4)
                            {
                                nb::Color color = nb::Color::fromLinearRgba(
                                    colorData[0], colorData[1], colorData[2], colorData[3]
                                );
                                material->setProperty(name, color);
                            }
                        }
                    }
                }
            }

            return material;
        }
        catch (const std::exception& e)
        {
            nb::Error::ErrorManager::instance().report(
                nb::Error::Type::FATAL, "Failed to load material: " + path.string() + " | " + e.what()
            );
            return nullptr;
        }
    }

    
}