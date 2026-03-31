// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "Engine.hpp"

#include "Math/RayCast/RayPicker.hpp"

        #include "Physics/Physics.hpp"

struct Ray
{
    nb::Math::Vector3<float> origin;
    nb::Math::Vector3<float> direction;
};

struct Plane
{
    nb::Math::Vector3<float> point;
    nb::Math::Vector3<float> normal;
};


bool rayIntersectsAABB(const Ray &ray, const nb::Math::Vector3<float> &boxMin, const nb::Math::Vector3<float> &boxMax)
{
    float tMin = -FLT_MAX, tMax = FLT_MAX;

    for (int i = 0; i < 3; ++i)
    {
        if (ray.direction[i] == 0.0f)
        {
            if (ray.origin[i] < boxMin[i] || ray.origin[i] > boxMax[i])
            {
                return false;
            }
        }
        else
        {
            float t1 = (boxMin[i] - ray.origin[i]) / ray.direction[i];
            float t2 = (boxMax[i] - ray.origin[i]) / ray.direction[i];

            if (t1 > t2)
                std::swap(t1, t2);

            tMin = (std::max)(tMin, t1);
            tMax = (std::min)(tMax, t2);

            if (tMin > tMax)
            {
                return false;
            }
        }
    }

    return true;
}


#include "../ECS/Ecs.hpp"
#include "../Loaders/PngLoader.hpp"
#include "Error/ErrorManager.hpp"
#include "Scripting/ScriptComponent.hpp"
extern "C"
{
    _declspec(dllexport) DWORD NvOptimusEnablement = 1;
}

namespace nb
{
    namespace Core
    {
        
        void Engine::bufferizeInput(const MSG &msg) const noexcept
        {
            uint32_t size = sizeof(RAWINPUT);
            static RAWINPUT rawInput;

            if (GetRawInputData(reinterpret_cast<HRAWINPUT>(msg.lParam),
                                RID_INPUT,
                                &rawInput,
                                &size,
                                sizeof(RAWINPUTHEADER)) == 0)
                return;

            input->bufferize(rawInput);
        }

        void Engine::processInput() const noexcept
        {
            input->updateAll();
            input->update();
        }

        void Engine::handleEditorMode() noexcept
        {
            using namespace nb::Input;
            input->stopHandlingPosition();

            ClipCursor(nullptr);

            if (keyboard->isKeyPressed(Keyboard::KeyCode::NB_1))
            {
                renderer->togglePolygonVisibilityMode(Renderer::PolygonMode::LINES);
            }
            if (keyboard->isKeyPressed(Keyboard::KeyCode::NB_2))
            {
                renderer->togglePolygonVisibilityMode(Renderer::PolygonMode::POINTS);
            }
            if (keyboard->isKeyPressed(Keyboard::KeyCode::NB_3))
            {
                nb::OpenGl::OpenGLRender::applyDefaultModel();
            }
            if (keyboard->isKeyPressed(Keyboard::KeyCode::NB_4))
            {
                nb::OpenGl::OpenGLRender::applyDefaultModelFlat();
            }
            if(keyboard->isKeyPressed(Keyboard::KeyCode::NB_9))
            {
                renderer->togglePolygonVisibilityMode(Renderer::PolygonMode::FULL);
            }
            if (keyboard->isKeyPressed(Keyboard::KeyCode::NB_CONTROL))
            {
                SetCursorPos(EngineSettings::getWidth() / 2, EngineSettings::getHeight() / 2);
            }
            if (keyboard->isKeyHeld(Keyboard::KeyCode::NB_CONTROL) && keyboard->isKeyPressed(Keyboard::KeyCode::NB_Z))
            {
                cam.toggleAlignByZ();
            }
            if (keyboard->isKeyHeld(Keyboard::KeyCode::NB_CONTROL) && keyboard->isKeyPressed(Keyboard::KeyCode::NB_Y))
            {
                cam.toggleAlignByY();
            }
            if (keyboard->isKeyHeld(Keyboard::KeyCode::NB_CONTROL) && keyboard->isKeyPressed(Keyboard::KeyCode::NB_X))
            {
                cam.toggleAlignByX();
            }
            if(keyboard->isKeyPressed(Keyboard::KeyCode::NB_TAB))
            {
                using namespace nb::ResMan;
                auto rm = ResourceManager::getInstance();
                const ResourceManager::ResourcePool& res = rm->getAllResources<nb::Renderer::Shader>();
                           
                for(auto&i : res)
                {
                    Resource::resourceTo<nb::Renderer::Shader>(i.second.get())->recompile();
                }
            }

        }

        bool Engine::run(bool shouldRender) 
        {
            using namespace nb::Input;
            processCommands();

            float deltaTime = Utils::Timer::timeElapsed();
             
            //this->mouseDelta = mouseDelta;
            renderer->setCamera(&cam);

           



            static float yaw;
            static float pitch;
            auto& scene = Scene::getInstance();
            auto& registry = scene.getRegistry();

            scene.traverseAll(
                [&](Ecs::EntityID entityId)
                {
                    Ecs::Entity entity{entityId};

                    if (registry.has<nb::Script::ScriptComponent>(entity))
                    {
                        auto& script = registry.get<nb::Script::ScriptComponent>(entity);
                        script.script->onUpdate(entity, deltaTime);

                    }
                }
            );


            cam.update(static_cast<float>(mouse->getX()), static_cast<float>(mouse->getY()));
            if (mode == Mode::GAME)
            {
                input->startHandlingPosition();
                input->startHandlingKeyboardEvents();
            }


            if (mode == Mode::GAME)
            {
                RECT r;
                GetClientRect(hwnd, &r);
                MapWindowPoints(hwnd, nullptr, reinterpret_cast<POINT *>(&r), 2);
                ClipCursor(&r);
            }


            if (keyboard->isKeyPressed(Keyboard::KeyCode::NB_ESCAPE))
            {
                SetCursorPos(EngineSettings::getWidth() / 2, EngineSettings::getHeight() / 2);

                if (mode == Mode::EDITOR)
                    mode = Mode::GAME;
                else
                    mode = Mode::EDITOR;
            }

            if (mode == Mode::EDITOR)
            {
                handleEditorMode();
            }
            else
            {
                auto camDir = cam.getDirection();
                handleGameMode(camDir, deltaTime);
            }

                renderer->render();
            if (shouldRender)
            {
            }

          
            return true;
        }

        void Engine::handleGameMode(nb::Math::Vector3<float> &camDir, float deltaTime) noexcept
        {
            using namespace nb::Input;

            PhysicsSystem physics;
            physics.update(Scene::getInstance(), deltaTime);


            if (keyboard->isKeyHeld(Keyboard::KeyCode::NB_S))
            {
                cam.moveAt(-camDir * 5.0f * deltaTime);
            }
            if (keyboard->isKeyHeld(Keyboard::KeyCode::NB_W))
            {
                cam.moveAt(camDir * 5.0f * deltaTime);
            }
            if (keyboard->isKeyHeld(Keyboard::KeyCode::NB_A))
            {
                auto rightVec = camDir.cross({0.0f, 1.0f, 0.0f});
                rightVec.normalize();
                cam.moveAt(-rightVec * 5.0f * deltaTime);
            }
            if (keyboard->isKeyHeld(Keyboard::KeyCode::NB_D))
            {
                auto rightVec = camDir.cross({0.0f, 1.0f, 0.0f});
                rightVec.normalize();
                cam.moveAt(rightVec * 5.0f * deltaTime);
            }
            if (keyboard->isKeyHeld(Keyboard::KeyCode::NB_SPACE))
            {
                cam.moveAt(cam.getUpVector() * 5.0f * deltaTime);
            }
            if (keyboard->isKeyHeld(Keyboard::KeyCode::NB_SHIFT))
            {
                cam.moveAt(-(cam.getUpVector() * 5.0f * deltaTime));
            }
        }

        Node Engine::rayPick(
            uint32_t x,
            uint32_t y
        ) const noexcept
        {
            // not impl 
            if (mode == Mode::GAME)
                return Node();

            Math::RayPicker picker;
            auto ray = picker.cast(
                renderer->getCamera(), x, y, EngineSettings::getWidth(),
                EngineSettings::getHeight()
            );

            auto result = nb::Scene::getInstance().pickNode(ray);

            if (result != 0)
            {
                Node selectedNode = nb::Scene::getInstance().getNode(result);
                
                nb::Error::ErrorManager::instance()
                    .report(nb::Error::Type::INFO, "Selected")
                    .with(
                        "Name", selectedNode.hasComponent<NameComponent>()
                                    ? selectedNode.getComponent<NameComponent>().name
                                    : "No name component"
                    );
                return selectedNode;

            }

            return Node();
        }

        Engine::Mode Engine::getMode() const noexcept
        {
            return mode;
        }

		NB_NODISCARD ShaderSystem& Engine::getShaderSystem() noexcept
		{
            return shaderSystem;
		}

	};
};
