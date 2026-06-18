#ifndef SRC_SCRIPTING_SCRIPTCOMPONENT_HPP
#define SRC_SCRIPTING_SCRIPTCOMPONENT_HPP

#include <Reflection/Reflection.hpp>
#include "ScriptEngine.hpp"

#include <string>
#include <memory>

namespace nb::Script
{
    class Script
    {
    public:
        Script(
            ScriptEngine& engine,
            const std::string& scriptPath
        );

        Script();

        void loadScript();

        void onUpdate(
            Ecs::Entity entity,
            float dt,
            const std::vector<std::pair<std::string, float>>& variables 
        )
        {
            if (!engine)
            {
                return;
            }

            for (const auto& var : variables)
            {
                env[var.first] = var.second;
            }

            sol::protected_function func = env["onUpdate"];
            if (!func.valid())
            {
                return;
            }

            auto result = func(entity.id, dt); 
            if (!result.valid())
            {
                sol::error err = result;
                nb::Error::ErrorManager::instance().report(nb::Error::Type::FATAL, err.what());
            }
        }

        template <typename... Args>
        void callFunction(
            std::string_view name,
            Args&&... arg
        )
        {
            if (!engine)
            {
                return;
            }

            sol::protected_function func = env[name];

            if (!func.valid())
            {
                return;
            }

            // Вызываем функцию
            auto result = func(std::forward<Args>(arg)...);

            if (!result.valid())
            {
                sol::error err = result;
                nb::Error::ErrorManager::instance().report(
                    nb::Error::Type::FATAL,
                    std::string("Lua Error in [") + std::string(name) + "]: " + err.what()
                );
            }
        }

        std::vector<std::string> getVariables() noexcept;


        void setEngine(ScriptEngine& eng);

        const std::string& getPath() const;

        Script(const Script& other);

        Script& operator=(const Script& other);

    public:
        ScriptEngine* engine; 
        std::string path;
        sol::environment env;
    };

    // struct ScriptVariable
    // {
    //     std::string name;
    //     float value;
    // };

    struct ScriptComponent
    {
        std::shared_ptr<Script> script; // NEVER MOVE FROM TOP, BECAUSE OF OFFSET 0
        std::vector<std::pair<std::string, float>> variables;// variant


        template <typename... Args>
        void call(
            std::string_view name,
            Args&&... args
        )
        {
            if (script)
            {
                script->callFunction(name, std::forward<Args>(args)...);
            }
        }

    };

};

// NB_REFLECT_STRUCT(
//     nb::Script::ScriptVariable,
//     NB_FIELD(
//         nb::Script::ScriptVariable,
//         name
//     ),
//     NB_FIELD(
//         nb::Script::ScriptVariable,
//         value
//     )
// )

NB_REFLECT_PTR(
    std::shared_ptr<nb::Script::Script>,
    "std::shared_ptr<nb::Script::Script>"
)

NB_REFLECT_RESOURCE_PTR(
    std::shared_ptr<nb::Script::Script>,
    "nb::Script::Script",
    [](std::shared_ptr<nb::Script::Script>* field,
       const std::string& path)
    {
        *field = std::make_shared<nb::Script::Script>(nb::Script::ScriptEngineSingleton::instance(), path); 
        
        auto* comp = reinterpret_cast<nb::Script::ScriptComponent*>(field);
        
        if (comp->script)
        {
            comp->script->loadScript(); 
            
            comp->variables.clear();
            std::vector<std::string> luaVars = comp->script->getVariables();
            
            for (const auto& varName : luaVars)
            {
                float initialValue = 0.0f;
                
                if (comp->script->env[varName].valid())
                {
                    sol::optional<float> optVal = comp->script->env[varName];
                    initialValue                = optVal.value_or(0.0f);
                }
                
                comp->variables.push_back({varName, initialValue});
            }
        }
    }
)

NB_REFLECT_STRUCT(
    nb::Script::ScriptComponent,
    NB_FIELD(
        nb::Script::ScriptComponent,
        script
    ),
    NB_FIELD(
        nb::Script::ScriptComponent,
        variables
    )
)


#endif