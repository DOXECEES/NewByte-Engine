#ifndef SRC_SCRIPTING_SCRIPTCOMPONENT_HPP
#define SRC_SCRIPTING_SCRIPTCOMPONENT_HPP

#include <string>

#include <Reflection/Reflection.hpp>
#include "ScriptEngine.hpp"

class Script
{
public:
    // Конструктор для создания с движком
    Script(
        ScriptEngine& engine,
        const std::string& scriptPath
    )
        : engine(&engine),
          path("Assets/res/" + scriptPath)
    {
        loadScript();
    }

    // Конструктор по пути (для десериализации без движка)
    Script() : engine(nullptr)
    {
    }

    // Загружаем скрипт, если есть движок
    void loadScript()
    {
        if (engine)
        {
            engine->loadScript(path);
        }
    }

    void update(float dt)
    {
        if (engine)
        {
            engine->callFunction("update", dt);
        }
    }

    // Устанавливаем движок после десериализации
    void setEngine(ScriptEngine& eng)
    {
        engine = &eng;
        loadScript();
    }

    // Получить путь к скрипту (для сериализации)
    const std::string& getPath() const
    {
        return path;
    }

    // Сделать копируемым только путь
    Script(const Script& other)
        : engine(nullptr),
          path(other.path)
    {
    }

    Script& operator=(const Script& other)
    {
        if (this != &other)
        {
            path = other.path;
            engine = nullptr; 
        }
        return *this;
    }

public:
    ScriptEngine* engine; 
    std::string path;
};


struct ScriptComponent
{
    Ref<Script> script;
};

NB_REFLECT_PTR(
    Ref<Script>,
    "Ref<Script>"
)

NB_REFLECT_RESOURCE_PTR(
    Ref<Script>,
    "Script",
    [](Ref<Script>* field,
       const std::string& path)
    {
        *field = createRef<Script>(ScriptEngineSingleton::instance(), path); 
    }
)

NB_REFLECT_STRUCT(
    ScriptComponent,
    NB_FIELD(
        ScriptComponent,
        script
    )
)


#endif