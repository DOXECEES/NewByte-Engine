#include "ScriptComponent.hpp"

namespace nb::Script
{

    Script::Script(
        ScriptEngine& engine,
        const std::string& scriptPath
    )
        : engine(&engine),
          path("Assets/res/" + scriptPath)
    {
        loadScript();
    }

    Script::Script() : engine(nullptr)
    {

    }

    void Script::loadScript()
    {
        if (engine)
        {
            engine->loadScript(path);
        }
    }

    //void Script::update(float dt)
    //{
    //    if (engine)
    //    {
    //        engine->callFunction("onUpdate", dt);
    //    }
    //}

    void Script::setEngine(ScriptEngine& eng)
    {
        engine = &eng;
        loadScript();
    }

    const std::string& Script::getPath() const
    {
        return path;
    }

    Script::Script(const Script& other)
        : engine(nullptr),
          path(other.path)
    {
    }

    Script& Script::operator=(const Script& other)
    {
        if (this != &other)
        {
            path = other.path;
            engine = nullptr;
        }
        return *this;
    }

}; 