#include "ScriptEngine.hpp"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyInterface.h>

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

    void ScriptEngine::registerEcsTypes()
    {
        auto& scene    = Scene::getInstance();
        auto& registry = scene.getRegistry();

        lua.new_usertype<Math::Vector3<float>>(
            "Vector3", sol::constructors<Math::Vector3<float>(float, float, float)>(), "new",
            sol::constructors<Math::Vector3<float>(float, float, float)>(), "x",
            &Math::Vector3<float>::x, "y", &Math::Vector3<float>::y, "z", &Math::Vector3<float>::z,
            "length", &Math::Vector3<float>::length, "normalize", &Math::Vector3<float>::normalize,
            sol::meta_function::addition,
            [](const Math::Vector3<float>& a, const Math::Vector3<float>& b)
            {
                return a + b;
            },
            sol::meta_function::subtraction,
            [](const Math::Vector3<float>& a, const Math::Vector3<float>& b)
            {
                return a - b;
            },
            sol::meta_function::multiplication,
            [](const Math::Vector3<float>& a, float b)
            {
                return a * b;
            }

        );

        lua.new_usertype<Math::Quaternion<float>>(
            "Quaternion", sol::constructors<Math::Quaternion<float>(float, float, float, float)>(),
            "new", sol::constructors<Math::Quaternion<float>(float, float, float, float)>(),
            "x", &Math::Quaternion<float>::x,
            "y", &Math::Quaternion<float>::y,
            "z", &Math::Quaternion<float>::z,
            "w", &Math::Quaternion<float>::w
        );

        lua.new_usertype<MeshComponent>(
            "MeshComponent",
            "isVisible", &MeshComponent::isVisible
        );


        lua.new_usertype<TransformComponent>(
            "TransformComponent", "position", &TransformComponent::position, "rotation",
            &TransformComponent::rotation, "scale", &TransformComponent::scale, "getPosition",
            [](TransformComponent& t)
            {
                return t.position;
            },
            "setPosition",
            [](TransformComponent& t, const Math::Vector3<float>& pos)
            {
                t.position     = pos;
                t.dirty        = true;
                t.physicsDirty = true;
            },
            "move",
            [](TransformComponent& t, float dx, float dy, float dz)
            {
                t.position.x += dx;
                t.position.y += dy;
                t.position.z += dz;
                t.dirty = true;
            },
            "rotate",
            [](TransformComponent& t, float dx, float dy, float dz)
            {
                auto deltaRot = nb::Math::Quaternion<float>::eulerToQuaternionXYZ(dx, dy, dz);

                t.rotation = t.rotation * deltaRot;
                t.rotation.normalize(); 

                t.dirty = true;
            },
            "getWorldPosition",
            [](TransformComponent& t)
            {
                return Math::getPositionFromModelMatrix(t.worldMatrix);
            }
        );

        lua.new_usertype<nb::Physics::Rigidbody>(
            "Rigidbody", "mass", &nb::Physics::Rigidbody::mass, "isStatic",
            &nb::Physics::Rigidbody::isStatic, "useGravity", &nb::Physics::Rigidbody::useGravity,
            "bodyID", &nb::Physics::Rigidbody::bodyID,

            "addForce", &nb::Physics::Rigidbody::addForce, "addTorque",
            &nb::Physics::Rigidbody::addTorque, "applyImpulse",
            &nb::Physics::Rigidbody::applyImpulse,
            "setPosition", [](nb::Physics::Rigidbody& rb, const Math::Vector3<float>& pos)
            {
                if (rb.bodyID.IsInvalid())
                {
                    return;
                }
                auto& bi = nb::Physics::PhysicsSystem::getInstance().getBodyInterface();

                bi.SetPosition(
                    rb.bodyID, JPH::RVec3(pos.x, pos.y, pos.z), JPH::EActivation::Activate
                );
            }
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
            },
            "getMeshComponent",
            [](nb::Scene& scene, uint32_t entityId) -> MeshComponent&
            {
                auto& registry = scene.getRegistry();
                if (!registry.has<MeshComponent>({entityId}))
                {
                    throw std::runtime_error("Entity has no MeshComponent");
                }
                return registry.get<MeshComponent>({entityId});
            },
            "findEntityByName",
            [](nb::Scene& scene, std::string_view name) -> uint32_t
            {
                return scene.findNodeByName(name).getId();
            },
            "getInstance",
            [](nb::Scene& scene) -> nb::Scene&
            {
                return scene;
            }
        );

        lua["Scene"]["getCameraComponent"] = [](nb::Scene& scene,
                                                uint32_t   entityId) -> CameraComponent&
        {
            auto& registry = scene.getRegistry();
            if (!registry.has<CameraComponent>({entityId}))
            {
                throw std::runtime_error("Entity has no CameraComponent");
            }
            return registry.get<CameraComponent>({entityId});
        };

        lua.new_usertype<CameraComponent>(
            "CameraComponent", "isPrimary", &CameraComponent::isPrimary, "getCamera",
            [](CameraComponent& cc)
            {
                return cc.controller.get();
            }
        );

        lua.new_usertype<nb::Renderer::Camera>(
            "Camera", "updateOrbit", &nb::Renderer::Camera::updateOrbit, "getDirection",
            &nb::Renderer::Camera::getDirection, "getPosition", &nb::Renderer::Camera::getPosition,
            "setDistance",
            [](nb::Renderer::Camera& c, float d)
            {
                c.setDistance(d);
            },
            "setTarget",
            [](nb::Renderer::Camera& c, const nb::Math::Vector3<float>& t)
            {
                c.setTarget(t);
            },
            "updateOrbitByAngles", &nb::Renderer::Camera::updateOrbitByAngles
        );

        lua.new_usertype<nb::Input::Mouse>(
            "Mouse", "getDeltaX",
            [](nb::Input::Mouse& m)
            {
                return m.getX();
            },
            "getDeltaY",
            [](nb::Input::Mouse& m)
            {
                return m.getY();
            },
            "getYaw",
            [](nb::Input::Mouse& m)
            {
                return m.getYaw();
            },
            "getPitch",
            [](nb::Input::Mouse& m)
            {
                return m.getPitch();
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

        lua.set_function(
            "print",
            [](sol::variadic_args args, sol::this_state s)
            {
                std::string     output;
                sol::state_view lua(s);

                for (auto it = args.begin(); it != args.end(); ++it)
                {
                    if (it != args.begin())
                    {
                        output += "\t";
                    }

                    std::string str = lua["tostring"](*it).get<std::string>();
                    output += str;
                }

                nb::Error::ErrorManager::instance().report(nb::Error::Type::WARNING, output);
            }
        );
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
                nb::Error::ErrorManager::instance().report(nb::Error::Type::FATAL, lastError
                );
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

            env.for_each(
                [&](sol::object key, sol::object value)
                {
                    if (value.is<sol::protected_function>())
                    {
                        // Здесь вы должны увидеть "onTriggerEnter" в логах
                        nb::Error::ErrorManager::instance().report(
                            nb::Error::Type::INFO,
                            "Script loaded function: " + key.as<std::string>()
                        );
                               
                    }
                }
            );


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

