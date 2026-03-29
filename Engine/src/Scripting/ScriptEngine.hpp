#pragma once
#include <filesystem>
#include <sol.hpp>
#include <string>
#include <unordered_map>

class ScriptEngine
{
public:
    ScriptEngine()
    {
        lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::package);
        
        
        lua.set_function(
            "print",
            [](sol::variadic_args args)
            {
                std::ostringstream logBuffer;

                for (auto arg : args)
                {
                    logBuffer << arg.as<std::string>() << " ";
                }
                logBuffer << "\n";

                nb::Error::ErrorManager::instance().report(nb::Error::Type::INFO, logBuffer.str());
            }
        );
    }

    bool loadScript(const std::filesystem::path& path)
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

    template <typename... Args>
    sol::protected_function_result callFunction(
        const std::string& name,
        Args&&... args
    )
    {
        sol::protected_function func = lua[name];
        if (!func.valid())
        {
            lastError = "Function not found: " + name;
            return sol::protected_function_result(); 
        }
        return func(std::forward<Args>(args)...);
    }

    std::string getLastError() const
    {
        return lastError;
    }

    sol::state& getLuaState()
    {
        return lua;
    }

private:
    sol::state lua;
    std::string lastError;
};

class ScriptEngineSingleton
{
public:
    static ScriptEngine& instance()
    {
        static ScriptEngine engine;
        return engine;
    }
};