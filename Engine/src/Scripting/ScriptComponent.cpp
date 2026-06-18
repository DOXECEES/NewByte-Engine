#include "ScriptComponent.hpp"

namespace nb::Script
{

    Script::Script(
        ScriptEngine& engine,
        const std::string& scriptPath
    )
        : engine(&engine),
          path(scriptPath)
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
            env = engine->createEnvironment();

            engine->loadScript(path, env);
        }
    }

    //void Script::update(float dt)
    //{
    //    if (engine)
    //    {
    //        engine->callFunction("onUpdate", dt);
    //    }
    //}

    std::vector<std::string> Script::getVariables() noexcept
    {
        std::vector<std::string> variables;

        if (!env.valid())
        {
            nb::Error::ErrorManager::instance().report(nb::Error::Type::FATAL, "Script envirironment is not valid");
            return variables;
        }

        for (auto const& pair : env)
        {
            sol::object key   = pair.first;
            sol::object value = pair.second;

            if (key.is<std::string>())
            {
                std::string varName = key.as<std::string>();

                if (!value.is<sol::function>() && !value.is<sol::table>())
                {
                    variables.push_back(std::move(varName));
                }
            }
        }

        return variables;
    }

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