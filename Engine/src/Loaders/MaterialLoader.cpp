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
            // 1. Загружаем и парсим JSON файл материала
            nb::Loaders::Json json;
            json.readFromFile(path);
            

            // 2. Создаем объект материала
            auto material = createRef<Resource::MaterialAsset>(path);

            // 3. Загружаем шейдер
           
           std::string shaderPath = json["shader"].get<std::string>();
           auto shader = nb::ResMan::ResourceManager::getInstance()->getResource<nb::Renderer::Shader>(
                        shaderPath
                    );




            material->setShader(shader);
            

            // 4. Парсим свойства (properties)
           
            {
                auto& propsNode = json["properties"];
                if (propsNode.isObject())
                {
                    for (auto const& [name, node] : propsNode.get<Node::Map>())
                    {
                        // Определяем тип данных в узле JSON
                        if (node.isValue())
                        {
                            // Если это строка — значит путь к текстуре
                            if (node.isString())
                            {
                                std::string texPath = node.get<std::string>();
                                auto texture =
                                    nb::ResMan::ResourceManager::getInstance()
                                        ->getResource<nb::Resource::TextureAsset>(texPath);

                                material->setTexture(name, texture);
                            }
                            // Если число — значит float параметр
                            else
                            {
                                // В твоем Node get<float>() должен уметь доставать числа
                                material->setProperty(name, node.get<float>());
                            }
                        }
                        // Если это массив — значит цвет (Vec4)
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
            // Логируем ошибку через твой ErrorManager
            nb::Error::ErrorManager::instance().report(
                nb::Error::Type::FATAL, "Failed to load material: " + path.string() + " | " + e.what()
            );
            return nullptr;
        }
    }

    
}