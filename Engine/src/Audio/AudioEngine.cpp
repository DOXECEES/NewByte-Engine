#define MINIAUDIO_IMPLEMENTATION
#include "AudioEngine.hpp"

AudioEngine::AudioEngine() : m_isInitialized(false)
{
}

AudioEngine::~AudioEngine()
{
    if (m_isInitialized)
    {
        ma_engine_uninit(&m_engine);
    }
}

bool AudioEngine::init()
{
    ma_result result = ma_engine_init(NULL, &m_engine);
    if (result != MA_SUCCESS)
    {
        std::cerr << "Failed to initialize audio engine" << std::endl;
        return false;
    }
    m_isInitialized = true;
    return true;
}

void AudioEngine::updateListener(
    const nb::Math::Vector3<float>& pos,
    const nb::Math::Vector3<float>& forward,
    const nb::Math::Vector3<float>& up
)
{
    ma_engine_listener_set_position(&m_engine, 0, pos.x, pos.y, pos.z);
    ma_engine_listener_set_direction(&m_engine, 0, forward.x, forward.y, forward.z);
    ma_engine_listener_set_world_up(&m_engine, 0, up.x, up.y, up.z);
}

void AudioEngine::play2D(
    const std::string& path,
    bool               looping,
    float              volume
)
{
    ma_uint32 flags = 0;
    if (looping)
    {
        flags |= MA_SOUND_FLAG_STREAM; // Стримим для зацикленных (обычно музыка)
    }

    // Простой запуск без управления объектом звука
    ma_engine_play_sound(&m_engine, path.c_str(), NULL);
}

void AudioEngine::setSoundPosition(
    ma_sound*        sound,
    const nb::Math::Vector3<float>& pos
)
{
    if (sound)
    {
        ma_sound_set_position(sound, pos.x, pos.y, pos.z);
    }
}


ma_sound* AudioEngine::load2DSound(
    const std::string& path,
    bool               looping
)
{
    ma_sound* sound = new ma_sound();
    // Используем MA_SOUND_FLAG_DECODE для быстрой работы или
    // MA_SOUND_FLAG_STREAM для длинных файлов (музыки)
    ma_uint32 flags = looping ? MA_SOUND_FLAG_STREAM : MA_SOUND_FLAG_DECODE;

    ma_result result = ma_sound_init_from_file(
        &m_engine,    // Указатель на движок
        path.c_str(), // Путь к файлу
        flags,        // Флаги
        NULL,         // Группа звуков (NULL - по умолчанию)
        NULL,         // Фенс для синхронизации (NULL - не нужен)
        sound         // Указатель на объект звука
    );

    if (result != MA_SUCCESS)
    {
        delete sound;
        return nullptr;
    }

    ma_sound_set_looping(sound, looping ? MA_TRUE : MA_FALSE);
    ma_sound_set_spatialization_enabled(sound, MA_FALSE); // Для 2D отключаем позиционирование
    return sound;
}

ma_sound* AudioEngine::load3DSound(
    const std::string& path,
    bool               looping
)
{
    ma_sound* sound = new ma_sound();
    ma_uint32 flags = looping ? MA_SOUND_FLAG_STREAM : MA_SOUND_FLAG_DECODE;

    ma_result result = ma_sound_init_from_file(
        &m_engine,    // Указатель на движок
        path.c_str(), // Путь к файлу
        flags,        // Флаги
        NULL,         // Группа звуков (NULL - по умолчанию)
        NULL,         // Фенс для синхронизации (NULL - не нужен)
        sound         // Указатель на объект звука
    );

    if (result != MA_SUCCESS)
    {
        delete sound;
        return nullptr;
    }

    ma_sound_set_looping(sound, looping ? MA_TRUE : MA_FALSE);
    ma_sound_set_spatialization_enabled(sound, MA_TRUE);
    return sound;
}

void AudioEngine::playSound(ma_sound* sound)
{
    if (sound && !ma_sound_is_playing(sound))
    {
        ma_sound_start(sound);
    }
}

void AudioEngine::pauseSound(ma_sound* sound)
{
    if (sound)
    {
        ma_sound_stop(sound); // В miniaudio stop работает как pause (сохраняет позицию)
    }
}

void AudioEngine::resumeSound(ma_sound* sound)
{
    if (sound)
    {
        ma_sound_start(sound);
    }
}

void AudioEngine::stopSound(ma_sound* sound)
{
    if (sound)
    {
        ma_sound_stop(sound);
        ma_sound_seek_to_pcm_frame(sound, 0); // Сбрасываем в начало
    }
}

void AudioEngine::restartSound(ma_sound* sound)
{
    if (sound)
    {
        ma_sound_seek_to_pcm_frame(sound, 0);
        ma_sound_start(sound);
    }
}

void AudioEngine::setVolume(
    ma_sound* sound,
    float     volume
)
{
    if (sound)
    {
        ma_sound_set_volume(sound, volume);
    }
}

bool AudioEngine::isPlaying(ma_sound* sound)
{
    return sound ? (bool)ma_sound_is_playing(sound) : false;
}

void AudioEngine::unloadSound(ma_sound* sound)
{
    if (sound)
    {
        ma_sound_uninit(sound);
        delete sound;
    }
}
