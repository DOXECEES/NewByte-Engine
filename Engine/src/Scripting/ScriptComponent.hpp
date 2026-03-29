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
            float dt
        )
        {
            if (!engine)
            {
                return;
            }

            sol::state& lua = engine->getLuaState();
            sol::protected_function func = lua["onUpdate"];
            if (!func.valid())
            {
                return;
            }

            auto result =
                func(entity.id, dt); 
            if (!result.valid())
            {
                sol::error err = result;
                nb::Error::ErrorManager::instance().report(nb::Error::Type::FATAL, err.what());
            }
        }


        void setEngine(ScriptEngine& eng);

        const std::string& getPath() const;

        Script(const Script& other);

        Script& operator=(const Script& other);

    public:
        ScriptEngine* engine; 
        std::string path;
    };


    struct ScriptComponent
    {
        std::shared_ptr<Script> script;
    };

};


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
    }
)

NB_REFLECT_STRUCT(
    nb::Script::ScriptComponent,
    NB_FIELD(
        nb::Script::ScriptComponent,
        script
    )
)


#endif