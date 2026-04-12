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


        void registerEcsTypes()
        {
            auto& scene = Scene::getInstance();
            auto& registry = scene.getRegistry();

            lua.new_usertype<Math::Vector3<float>>(
                "Vector3",
                sol::constructors<
                    Math::Vector3<float>(float, float, float)
                >(),
                "new",
                sol::constructors<Math::Vector3<float>(float, float, float)>(),
                "x", &Math::Vector3<float>::x,
                "y", &Math::Vector3<float>::y,
                "z", &Math::Vector3<float>::z,
                "length", &Math::Vector3<float>::length,
                "normalize", &Math::Vector3<float>::normalize,
                sol::meta_function::addition,
                [](const Math::Vector3<float>& a, const Math::Vector3<float>& b)
                {
                    return a + b;
                }
            );

            lua.new_usertype<TransformComponent>(
                "TransformComponent",
                "position", &TransformComponent::position, "rotation",
                &TransformComponent::rotation, "scale", &TransformComponent::scale,
                "getPosition",
                [](TransformComponent& t)
                {
                    return t.position;
                },
                "setPosition",
                [](TransformComponent& t, const Math::Vector3<float>& pos)
                {
                    t.position = pos;
                },
                "move",
                [](TransformComponent& t, float dx, float dy, float dz)
                {
                    t.position.x += dx;
                    t.position.y += dy;
                    t.position.z += dz;
                    t.dirty = true;
                }
            );

            lua.new_usertype<nb::Physics::Rigidbody>(
                "Rigidbody",
                "mass", &nb::Physics::Rigidbody::mass,
                "isStatic", &nb::Physics::Rigidbody::isStatic,
                "useGravity", &nb::Physics::Rigidbody::useGravity,
                "bodyID", &nb::Physics::Rigidbody::bodyID,
               
                "addForce", &nb::Physics::Rigidbody::addForce,
                "addTorque", &nb::Physics::Rigidbody::addTorque,
                "applyImpulse", &nb::Physics::Rigidbody::applyImpulse
            );


            lua.new_usertype<nb::Scene>(
                "Scene", "getTransform",
                [](nb::Scene& scene, uint32_t entityId) -> TransformComponent&
                {
                    auto& registry = scene.getRegistry();
                    if (!registry.has<TransformComponent>({entityId}))
                    {
                        throw std::runtime_error(
                            "Entity " + std::to_string(entityId) + " has no TransformComponent"
                        );
                    }
                    return registry.get<TransformComponent>({entityId});
                },
                "getRigidbody",
                [](nb::Scene& scene, uint32_t entityId) -> nb::Physics::Rigidbody&
                {
                    auto& registry = scene.getRegistry();
                    if (!registry.has<nb::Physics::Rigidbody>({entityId}))
                    {
                        throw std::runtime_error("Entity has no RigidbodyComponent");
                    }
                    return registry.get<nb::Physics::Rigidbody>({entityId});
                }
            );



            lua.new_usertype<nb::Input::Keyboard>(
                "Keyboard", "isKeyHeld",
                [](nb::Input::Keyboard& input, const std::string& key)
                {
                    nb::Input::Keyboard::KeyCode code;
                    if (key == "W")
                    {
                        code = nb::Input::Keyboard::KeyCode::NB_W;
                    }
                    else if (key == "A")
                    {
                        code = nb::Input::Keyboard::KeyCode::NB_A;
                    }
                    else if (key == "S")
                    {
                        code = nb::Input::Keyboard::KeyCode::NB_S;
                    }
                    else if (key == "D")
                    {
                        code = nb::Input::Keyboard::KeyCode::NB_D;
                    }
                    else if (key == "F")
                    {
                        code = nb::Input::Keyboard::KeyCode::NB_F;
                    }
                    else if (key == "SPACE")
                    {
                        code = nb::Input::Keyboard::KeyCode::NB_SPACE;
                    }
                    else if (key == "SHIFT")
                    {
                        code = nb::Input::Keyboard::KeyCode::NB_SHIFT;
                    }
                    else
                    {
                        return false;
                    }

                    return input.isKeyHeld(code);
                }
            );

           
            

        }

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