#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#include "../../dependencies/Audio/miniaudio.h"
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "Math/Vector3.hpp"
#include "ECS/Ecs.hpp"
#include "Renderer/Camera.hpp"
#include "Renderer/Scene.hpp"

#include <Reflection/Reflection.hpp>

struct AudioComponent
{
    ma_sound*   soundHandle = nullptr;
    
    std::filesystem::path path;
    bool        is3D        = true;
    bool        loop        = false;
    float       volume      = 1.0f;
    bool        shouldPlay  = false;
};

NB_REFLECT_STRUCT(
    AudioComponent,
    NB_FIELD(
        AudioComponent,
        path
    ),
    NB_FIELD(
        AudioComponent,
        is3D
    ),
    NB_FIELD(
        AudioComponent,
        loop
    ),
    NB_FIELD(
        AudioComponent,
        volume
    ),
    NB_FIELD(
        AudioComponent,
        shouldPlay
    )
)


class AudioEngine
{
public:
    AudioEngine();
    ~AudioEngine();

    bool init();
    void updateListener(
        const nb::Math::Vector3<float>& pos,
        const nb::Math::Vector3<float>& forward,
        const nb::Math::Vector3<float>& up
    );

    void play2D(
        const std::string& path,
        bool               looping = false,
        float              volume  = 1.0f
    );

    ma_sound* load3DSound(
        const std::string& path,
        bool               looping = false
    );

    void setSoundPosition(
        ma_sound*                       sound,
        const nb::Math::Vector3<float>& pos
    );

    void playSound(ma_sound* sound);
    void stopSound(ma_sound* sound);

    ma_sound* load2DSound(
        const std::string& path,
        bool               looping = false
    );

    // Управление состоянием
    void pauseSound(ma_sound* sound);
    void resumeSound(ma_sound* sound);
    void restartSound(ma_sound* sound); // Сброс на начало и проигрывание
    void setVolume(
        ma_sound* sound,
        float     volume
    );
    bool isPlaying(ma_sound* sound);

    // Освобождение памяти
    void unloadSound(ma_sound* sound);

private:
    ma_engine m_engine;
    bool      m_isInitialized = false;
};

class AudioSystem
{
public:
    void update(
        std::vector<nb::Ecs::Entity>& entities,
        AudioEngine&         engine,
        const nb::Renderer::Camera&        cam
    )
    {
        // 1. Обновляем слушателя
        engine.updateListener(cam.getPosition(), cam.getDirection(), cam.getUpVector());

        for (auto& entity : entities)
        {
            auto& registry  = nb::Scene::getInstance().getRegistry();
            auto& audio     = registry.get<AudioComponent>(entity);
            auto& transform = registry.get<TransformComponent>(entity);

            // Если звук еще не загружен — загружаем
            if (!audio.soundHandle)
            {
                audio.soundHandle = audio.is3D ? engine.load3DSound(audio.path.string(), audio.loop)
                                               : engine.load2DSound(audio.path.string(), audio.loop);
            }

            // Если звук 3D — обновляем его позицию в мире
            if (audio.is3D)
            {
                engine.setSoundPosition(audio.soundHandle, transform.position);
            }

            // Управление воспроизведением
            if (audio.shouldPlay && !engine.isPlaying(audio.soundHandle))
            {
                engine.playSound(audio.soundHandle);
            }
            else if (!audio.shouldPlay && engine.isPlaying(audio.soundHandle))
            {
                engine.pauseSound(audio.soundHandle);
            }

            engine.setVolume(audio.soundHandle, audio.volume);
        }
    }
};


#endif