#include "ScriptEngine.hpp"

namespace nb::Script
{
    ScriptEngine::ScriptEngine()
    {
        lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::package);
        registerEcsTypes();
        lua["Scene"] = &nb::Scene::getInstance();

    }

    sol::environment ScriptEngine::createEnvironment()
    {
        sol::environment env(lua, sol::create, lua.globals());
        return env;
    }

    bool ScriptEngine::loadScript(
        const std::filesystem::path& path,
        sol::environment&            env
    )
    {
        try
        {
            sol::load_result script_load = lua.load_file(path.string());

            if (!script_load.valid())
            {
                sol::error err = script_load;
                lastError      = "Load Error: " + std::string(err.what());
                return false;
            }

            sol::protected_function script_func = script_load;

            sol::set_environment(env, script_func);

            auto result = script_func();
            if (!result.valid())
            {
                sol::error err = result;
                lastError      = "Runtime Error: " + std::string(err.what());
                return false;
            }

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

