#ifndef SRC_SCRIPTING_SCRIPTENGINE_HPP
#define SRC_SCRIPTING_SCRIPTENGINE_HPP
#include<filesystem>
#include <sol.hpp>
#include <string>
#include <unordered_map>


#include "Renderer/Scene.hpp"
#include "Renderer/Renderer.hpp"

#include "Input/Input.hpp"

#include "Physics/Physics.hpp"

namespace nb::Script
{
    class ScriptEngine
    {
    public:
        ScriptEngine();

        sol::environment createEnvironment();


        void registerEcsTypes();
        

        bool runScript(const std::string& path)
        {
            sol::load_result script = lua.load_file(path);
            if (!script.valid())
            {
                sol::error err = script;
                printf("Lua load error: %s\n", err.what());
                return false;
            }

            sol::protected_function_result result = script();
            if (!result.valid())
            {
                sol::error err = result;
                printf("Lua runtime error: %s\n", err.what());
                return false;
            }
            return true;
        }
        bool loadScript(
            const std::filesystem::path& path,
            sol::environment&            env
        );

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

        std::string getLastError() const;

        sol::state& getLuaState();

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
}

#endif