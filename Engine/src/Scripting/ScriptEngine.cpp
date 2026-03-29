#include "ScriptEngine.hpp"

namespace nb::Script
{
    ScriptEngine::ScriptEngine()
    {
        lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::package);
        registerEcsTypes();
        lua["Scene"] = &nb::Scene::getInstance();

    }

    bool ScriptEngine::loadScript(const std::filesystem::path& path)
    {
        try
        {
            sol::load_result script = lua.load_file(path.string());
            if (!script.valid())
            {
                sol::error err = script;
                lastError = err.what();
                return false;
            }
            script();
            return true;
        }
        catch (const sol::error& e)
        {
            lastError = e.what();
            return false;
        }
    }
    std::string ScriptEngine::getLastError() const
    {
        return lastError;
    }
    sol::state& ScriptEngine::getLuaState()
    {
        return lua;
    }
};

