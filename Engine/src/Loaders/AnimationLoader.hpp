#include "Loaders/JSON/Json.hpp"
#include "Renderer/Animation.hpp"
#include "Manager/ResourceManager.hpp"

#include <string>
#include <filesystem>

namespace nb::Resource
{

    Ref<Renderer::Animation> loadAnimation(const std::filesystem::path& animPath)
    {
        // 1. Читаем дескриптор .anim
        auto animJson = nb::Loaders::Json(animPath);

        std::string sourcePath = animJson["source"].get<std::string>();
        std::string meshPath   = animJson["mesh"].get<std::string>();
        int         trackIndex = animJson.contains("track_index") ? animJson["track_index"].get<int>() : 0;

        // 2. Получаем меш через менеджер ресурсов (он гарантированно вернет готовый меш из кэша)
        auto mesh = nb::ResMan::ResourceManager::getInstance()->getResource<Renderer::Mesh>(meshPath);
        if (!mesh)
        {
            return nullptr;
        }

        // 3. Создаем анимацию
        // Для поддержки выбора трека по индексу мы можем передать trackIndex в конструктор Animation
        auto animation = std::make_shared<Renderer::Animation>(sourcePath, mesh, trackIndex);


        return animation;
    }
}