#include "Engine.hpp"

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
namespace nb
{
    namespace Core
    {
        struct Helth
        {
            int i=0;
        };

        struct Speed
        {
            int i = 0;
        };

        Engine::Engine(const HWND &hwnd)
        {

#ifdef NB_DEBUG

            Math::Vector3<float> v1 = {0.0f, 0.0f, 0.0f};
            Math::Vector3<float> v2 = {0.0f, 1.0f, 0.0f};

            Math::Ray r(v1, v2);

            Math::Ray r2({5.0f, 2.0f, 0.0f}, {-1.0f, 0.0f, 0.0f});
            auto res = Math::distanceBetweenRays(r, r2);

            //auto v = nb::Math::projectVectorToVector(v1, v2);


            //Loaders::PngLoader loader("C:\\Users\\isymo\\Pictures\\Screenshots\\1.png");
            

            Ecs::EcsManager manager;
            auto ent1 = manager.createEntity();
            ent1.add<Helth>(Helth(1));
            ent1.add<Speed>(Speed(1));
            auto ent2 = manager.createEntity();
            ent2.add<Helth>(Helth(2));

            Debug::debug("HELTH:");

            for(auto&i : manager.getEntitiesWithComponent<Helth>())
            {
                auto h = i.get<Helth>();
                Debug::debug(h.i);
            }
            Debug::debug("Speed:");

            for(auto&i : manager.getEntitiesWithComponent<Speed>())
            {
                auto h = i.get<Speed>();
                Debug::debug(h.i);
            }
#endif            


            subSystems->Init(hwnd);
            this->hwnd = hwnd;
            keyboard = subSystems->getKeyboard();
            mouse = subSystems->getMouse();
            input = createRef<Input::Input>();
            input->linkKeyboard(keyboard);
            input->linkMouse(mouse);
            renderer = subSystems->getRenderer();
            renderer->setCamera(&cam);
            Utils::Timer::init();   
        }

        void Engine::bufferizeInput(const MSG &msg) noexcept
        {
            uint32_t size = sizeof(RAWINPUT);
            static RAWINPUT rawInput;
            MSG nMsg;

            if (GetRawInputData(reinterpret_cast<HRAWINPUT>(msg.lParam), RID_INPUT, &rawInput, &size, sizeof(RAWINPUTHEADER)) == 0)
                return;

            input->bufferize(rawInput);
        }

        void Engine::processInput() noexcept
        {
            input->updateAll();
            input->update();
        }

        void Engine::handleEditorMode() noexcept
        {
            input->stopHandlingPosition();

            // show cursor
            ClipCursor(nullptr);

            if (keyboard->isKeyPressed(0x31))
            {
                renderer->togglePolygonVisibilityMode(Renderer::Renderer::PolygonMode::LINES);
            }
            if (keyboard->isKeyPressed(0x32))
            {
                renderer->togglePolygonVisibilityMode(Renderer::Renderer::PolygonMode::POINTS);
            }
            if (keyboard->isKeyPressed(0x33))
            {
                nb::OpenGl::OpenGLRender::applyDefaultModel();
            }
            if (keyboard->isKeyPressed(0x34))
            {
                nb::OpenGl::OpenGLRender::applyDefaultModelFlat();
            }
            if(keyboard->isKeyPressed(0x39))
            {
                renderer->togglePolygonVisibilityMode(Renderer::Renderer::PolygonMode::FULL);
            }
            if (keyboard->isKeyPressed(VK_CONTROL))
            {
                SetCursorPos(EngineSettings::getWidth() / 2, EngineSettings::getHeight() / 2);
            }
            if (keyboard->isKeyHeld(VK_CONTROL) && keyboard->isKeyPressed(0x5A))
            {
                cam.toggleAlignByZ();
            }
            if (keyboard->isKeyHeld(VK_CONTROL) && keyboard->isKeyPressed(0x59))
            {
                cam.toggleAlignByY();
            }
            if (keyboard->isKeyHeld(VK_CONTROL) && keyboard->isKeyPressed(0x58))
            {
                cam.toggleAlignByX();
            }
            if(keyboard->isKeyPressed(VK_TAB))
            {
                using namespace nb::ResMan;
                auto rm = ResourceManager::getInstance();
                const ResourceManager::ResourcePool& res = rm->getAllResources<nb::Renderer::Shader>();
                
                for(auto&i : res)
                {
                    std::dynamic_pointer_cast<nb::OpenGl::OpenGlShader>(i.second)->recompile();
                }
            }
        }

        bool Engine::run(Input::MouseDelta mouseDelta, Input::MouseButtons buttons) 
        {
            float deltaTime = Utils::Timer::timeElapsed();
             
            this->mouseDelta = mouseDelta;
            renderer->setCamera(&cam);

            static float yaw;
            static float pitch;


            cam.update(mouse->getX(), mouse->getY());
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

            static float xAngle = 0.0f;
            static float yAngle = 0.0f;
            static float zAngle = 0.0f;

            auto camPos = cam.getPosition();
            auto camDir = cam.getDirection();

            if (keyboard->isKeyPressed(VK_ESCAPE))
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
                handleGameMode(camDir, deltaTime);
            }

            renderer->render();
            return true;
        }

        void Engine::handleGameMode(nb::Math::Vector3<float> &camDir, float deltaTime) noexcept
        {
            if (keyboard->isKeyHeld(0x53))
            {
                cam.moveAt(camDir * 2.0f * deltaTime);
            }
            if (keyboard->isKeyHeld(0x57))
            {
                cam.moveAt(-camDir * 2.0f * deltaTime);
            }
            if (keyboard->isKeyHeld(0x41))
            {
                auto rightVec = camDir.cross({0.0f, 1.0f, 0.0f});
                rightVec.normalize();
                cam.moveAt(rightVec * 2.0f * deltaTime);
            }
            if (keyboard->isKeyHeld(0x44))
            {
                auto rightVec = camDir.cross({0.0f, 1.0f, 0.0f});
                rightVec.normalize();
                cam.moveAt(-rightVec * 2.0f * deltaTime);
            }
            if (keyboard->isKeyHeld(VK_SPACE))
            {
                cam.moveAt(cam.getUpVector() * 2.0f * deltaTime);
            }
            if (keyboard->isKeyHeld(VK_SHIFT))
            {
                cam.moveAt(-(cam.getUpVector() * 2.0f * deltaTime));
            }
        }

        void Engine::rayPick(const uint32_t x, const uint32_t y) noexcept
        {
            if (mode == Mode::GAME)
                return;

            nb::Math::RayPicker rp;
            auto a = rp.cast(renderer->getCamera(), x, y, EngineSettings::getWidth(), EngineSettings::getHeight());
            auto scene = renderer->getScene()->getScene();
            

            std::stack<std::shared_ptr<Renderer::BaseNode>> stk;
            stk.push(scene);
            bool interFind = false;

            while (!stk.empty())
            {
                std::shared_ptr<Renderer::BaseNode> top = stk.top();
                stk.pop();
                for (auto& i : top->getChildrens())
                {
                    stk.push(i);
                }


                if (auto n = std::dynamic_pointer_cast<Renderer::ObjectNode>(top); n != nullptr)
                {
                    auto aabb = n->mesh->getAabb3d();
                    auto naabb = Math::AABB3D::recalculateAabb3dByModelMatrix(aabb, n->getWorldTransform());
                    
                    auto camPos = renderer->getCamera()->getPosition();
                    Ray ray;
                    ray.direction = a;
                    ray.origin = camPos;
                    

                    if (rayIntersectsAABB(ray, naabb.minPoint, naabb.maxPoint))
                    {
                        OpenGl::OpenGLRender::setAmbientLight({255, 120, 100});
                        interFind = true;
                        nb::OpenGl::OpenGLRender::spawnGizmo(*n);

                        if (buttons & 0x0001)
                        {
                            OpenGl::OpenGLRender::setAmbientLight({255, 0, 0});
                        }   
                    }
                    else
                    {
                        if(interFind == false)
                            OpenGl::OpenGLRender::setAmbientLight({0, 0, 0});
                    }
                }

                
                
            }


           
        }

        Engine::Mode Engine::getMode() const noexcept
        {
            return mode;
        }

    };
};
