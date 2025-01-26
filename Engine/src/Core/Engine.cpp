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

nb::Math::Vector2<float> projectToScreen(const nb::Math::Vector3<float> &worldPos, const nb::Math::Mat4<float> &projection, const nb::Math::Mat4<float> &view, const nb::Math::Vector2<int> &screenSize)
{
    auto clipSpacePos = projection * view * nb::Math::Vector4<float>(worldPos.x, worldPos.y, worldPos.z, 1.0f);
    auto ndc = nb::Math::Vector3<float>(clipSpacePos.x, clipSpacePos.y, clipSpacePos.z);

    // Convert NDC to screen space (assuming NDC range [-1, 1] and screen space range [0, screenSize])
    nb::Math::Vector2<float> screenPos;
    screenPos.x = (ndc.x + 1.0f) * 0.5f * screenSize.x;
    screenPos.y = (1.0f - ndc.y) * 0.5f * screenSize.y;

    return screenPos;
}

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

bool intersect(const Ray &ray, const Plane &plane, const nb::Math::Vector3<float> &axisStart,
               const nb::Math::Vector3<float> &axisEnd, const nb::Math::Mat4<float> &proj, const nb::Math::Mat4<float> &view,
               const nb::Math::Vector2<int> &screenSize, const nb::Math::Vector2<float> &mousePos, float pixelThreshold, nb::Math::Vector3<float> &delta)
{

    float denom = plane.normal.dot(ray.direction);

    // If the ray is parallel to the plane (denom == 0), no intersection
    if (denom == 0.0f)
    {
        return false;
    }

    // Calculate t using the formula derived above
    auto t = plane.normal.dot(plane.point - ray.origin) / denom;

    // If t is negative, the intersection is behind the ray's origin (optional check)
    if (t < 0.0f)
    {
        return false;
    }

    // Calculate the intersection point
    auto intersectionPoint = ray.origin + (ray.direction * t);

    nb::Math::Vector2<float> projectedStart = projectToScreen(axisStart, proj, view, screenSize);
    nb::Math::Vector2<float> projectedEnd = projectToScreen(axisEnd, proj, view, screenSize);
    nb::Math::Vector2<float> projectedIntersection = projectToScreen(intersectionPoint, proj, view, screenSize);

    // Calculate the distance between the mouse position and the projected x-axis segment
    float distance = projectedStart.distance(projectedEnd);
    float distToSegment = projectedIntersection.distance(mousePos);

    // Check if the mouse is close enough to the projected segment
    if (distToSegment < pixelThreshold)
    {
        // Calculate the delta vector in world space (difference between selection's center and intersection)
        delta = intersectionPoint - nb::Math::Vector3<float>(0.0f, 0.0f, 0.0f);
        return true;
    }

    return false;
}

namespace nb
{
    namespace Core
    {
        Engine::Engine(const HWND &hwnd)
        {
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

        void Engine::processInput(const MSG &msg) noexcept
        {
            input->update(msg);
        }

        bool Engine::run()
        {
            //Debug::debug(Utils::Timer::timeElapsed());
            
            //auto n  =  Math::inverse(m);

            renderer->setCamera(&cam);

            static float yaw;
            static float pitch;

            if (isEditorMode)
            {
                if (mouse->isLeftClick())
                {
                    Debug::debug("clk");
                }
            }

            cam.update(mouse->getX(), mouse->getY());
            if (!isEditorMode)
            {
                input->startHandlingPosition();
                input->startHandlingKeyboardEvents();
            }
            else
            {
                input->stopHandlingPosition();
                // input->stopHandlingKeyboardEvents();
            }


            if (!isEditorMode)
            {
                RECT r;
                GetClientRect(hwnd, &r);
                MapWindowPoints(hwnd, nullptr, reinterpret_cast<POINT *>(&r), 2);
                ClipCursor(&r);
            }
            else
            {
                ClipCursor(nullptr);
            }

            



            std::vector<Renderer::Vertex> vert{
                {{-0.934172, -0.356822, 0.000000}, {-0.525700, -0.850700, -0.000000}}, {{-0.577350, -0.577350, -0.577350}, {-0.525700, -0.850700, -0.000000}}, {{-0.417775, -0.675973, 0.000000}, {-0.525700, -0.850700, -0.000000}}, {{0.000000, -0.934172, -0.356822}, {-0.525700, -0.850700, -0.000000}}, {{0.000000, -0.934172, 0.356822}, {-0.525700, -0.850700, -0.000000}}, {{-0.577350, -0.577350, 0.577350}, {-0.525700, -0.850700, -0.000000}}, {{-0.356822, 0.000000, 0.934172}, {-0.000000, 0.525700, 0.850700}}, {{0.356822, 0.000000, 0.934172}, {-0.000000, 0.525700, 0.850700}}, {{0.000000, 0.417775, 0.675973}, {-0.000000, 0.525700, 0.850700}}, {{0.577350, 0.577350, 0.577350}, {-0.000000, 0.525700, 0.850700}}, {{0.000000, 0.934172, 0.356822}, {-0.000000, 0.525700, 0.850700}}, {{-0.577350, 0.577350, 0.577350}, {-0.000000, 0.525700, 0.850700}}, {{-0.577350, 0.577350, -0.577350}, {-0.525700, 0.850700, -0.000000}}, {{-0.934172, 0.356822, 0.000000}, {-0.525700, 0.850700, -0.000000}}, {{-0.417775, 0.675973, -0.000000}, {-0.525700, 0.850700, -0.000000}}, {{-0.577350, 0.577350, 0.577350}, {-0.525700, 0.850700, -0.000000}}, {{0.000000, 0.934172, 0.356822}, {-0.525700, 0.850700, -0.000000}}, {{0.000000, 0.934172, -0.356822}, {-0.525700, 0.850700, -0.000000}}, {{0.000000, -0.934172, 0.356822}, {-0.000000, -0.525700, 0.850700}}, {{0.577350, -0.577350, 0.577350}, {-0.000000, -0.525700, 0.850700}}, {{-0.000000, -0.417775, 0.675973}, {-0.000000, -0.525700, 0.850700}}, {{0.356822, 0.000000, 0.934172}, {-0.000000, -0.525700, 0.850700}}, {{-0.356822, 0.000000, 0.934172}, {-0.000000, -0.525700, 0.850700}}, {{-0.577350, -0.577350, 0.577350}, {-0.000000, -0.525700, 0.850700}}, {{-0.356822, 0.000000, 0.934172}, {-0.850700, -0.000000, 0.525700}}, {{-0.577350, 0.577350, 0.577350}, {-0.850700, -0.000000, 0.525700}}, {{-0.675973, -0.000000, 0.417775}, {-0.850700, -0.000000, 0.525700}}, {{-0.934172, 0.356822, 0.000000}, {-0.850700, -0.000000, 0.525700}}, {{-0.934172, -0.356822, 0.000000}, {-0.850700, -0.000000, 0.525700}}, {{-0.577350, -0.577350, 0.577350}, {-0.850700, -0.000000, 0.525700}}, {{0.356822, 0.000000, -0.934172}, {-0.000000, 0.525700, -0.850700}}, {{-0.356822, 0.000000, -0.934172}, {-0.000000, 0.525700, -0.850700}}, {{-0.000000, 0.417775, -0.675973}, {-0.000000, 0.525700, -0.850700}}, {{-0.577350, 0.577350, -0.577350}, {-0.000000, 0.525700, -0.850700}}, {{0.000000, 0.934172, -0.356822}, {-0.000000, 0.525700, -0.850700}}, {{0.577350, 0.577350, -0.577350}, {-0.000000, 0.525700, -0.850700}}, {{-0.577350, -0.577350, -0.577350}, {-0.850700, -0.000000, -0.525700}}, {{-0.934172, -0.356822, 0.000000}, {-0.850700, -0.000000, -0.525700}}, {{-0.675973, 0.000000, -0.417775}, {-0.850700, -0.000000, -0.525700}}, {{-0.934172, 0.356822, 0.000000}, {-0.850700, -0.000000, -0.525700}}, {{-0.577350, 0.577350, -0.577350}, {-0.850700, -0.000000, -0.525700}}, {{-0.356822, 0.000000, -0.934172}, {-0.850700, -0.000000, -0.525700}}, {{0.000000, -0.934172, 0.356822}, {0.525700, -0.850700, -0.000000}}, {{0.000000, -0.934172, -0.356822}, {0.525700, -0.850700, -0.000000}}, {{0.417775, -0.675973, -0.000000}, {0.525700, -0.850700, -0.000000}}, {{0.577350, -0.577350, -0.577350}, {0.525700, -0.850700, -0.000000}}, {{0.934172, -0.356822, 0.000000}, {0.525700, -0.850700, -0.000000}}, {{0.577350, -0.577350, 0.577350}, {0.525700, -0.850700, -0.000000}}, {{0.577350, 0.577350, 0.577350}, {0.850700, -0.000000, 0.525700}}, {{0.356822, 0.000000, 0.934172}, {0.850700, -0.000000, 0.525700}}, {{0.675973, 0.000000, 0.417775}, {0.850700, -0.000000, 0.525700}}, {{0.577350, -0.577350, 0.577350}, {0.850700, -0.000000, 0.525700}}, {{0.934172, -0.356822, 0.000000}, {0.850700, -0.000000, 0.525700}}, {{0.934172, 0.356822, 0.000000}, {0.850700, -0.000000, 0.525700}}, {{0.934172, 0.356822, 0.000000}, {0.850700, -0.000000, -0.525700}}, {{0.934172, -0.356822, 0.000000}, {0.850700, -0.000000, -0.525700}}, {{0.675973, -0.000000, -0.417775}, {0.850700, -0.000000, -0.525700}}, {{0.577350, -0.577350, -0.577350}, {0.850700, -0.000000, -0.525700}}, {{0.356822, 0.000000, -0.934172}, {0.850700, -0.000000, -0.525700}}, {{0.577350, 0.577350, -0.577350}, {0.850700, -0.000000, -0.525700}}, {{0.000000, 0.934172, -0.356822}, {0.525700, 0.850700, -0.000000}}, {{0.000000, 0.934172, 0.356822}, {0.525700, 0.850700, -0.000000}}, {{0.417775, 0.675973, 0.000000}, {0.525700, 0.850700, -0.000000}}, {{0.577350, 0.577350, 0.577350}, {0.525700, 0.850700, -0.000000}}, {{0.934172, 0.356822, 0.000000}, {0.525700, 0.850700, -0.000000}}, {{0.577350, 0.577350, -0.577350}, {0.525700, 0.850700, -0.000000}}, {{0.577350, -0.577350, -0.577350}, {-0.000000, -0.525700, -0.850700}}, {{0.000000, -0.934172, -0.356822}, {-0.000000, -0.525700, -0.850700}}, {{0.000000, -0.417775, -0.675973}, {-0.000000, -0.525700, -0.850700}}, {{-0.577350, -0.577350, -0.577350}, {-0.000000, -0.525700, -0.850700}}, {{-0.356822, 0.000000, -0.934172}, {-0.000000, -0.525700, -0.850700}}, {{0.356822, 0.000000, -0.934172}, {-0.000000, -0.525700, -0.850700}}};

            std::vector<uint32_t> ind{
                0,1,2,1,3,2,3,4,2,4,5,2,5,0,2,6,7,8,7,9,8,9,10,8,10,11,8,11,6,8,12,13,14,13,15,14,15,16,14,16,17,14,17,12,14,18,19,20,19,21,20,21,22,20,22,23,20,23,18,20,24,25,26,25,27,26,27,28,26,28,29,26,29,24,26,30,31,32,31,33,32,33,34,32,34,35,32,35,30,32,36,37,38,37,39,38,39,40,38,40,41,38,41,36,38,42,43,44,43,45,44,45,46,44,46,47,44,47,42,44,48,49,50,49,51,50,51,52,50,52,53,50,53,48,50,54,55,56,55,57,56,57,58,56,58,59,56,59,54,56,60,61,62,61,63,62,63,64,62,64,65,62,65,60,62,66,67,68,67,69,68,69,70,68,70,71,68,71,66,68};

            // :)

            static float xAngle = 0.0f;
            static float yAngle = 0.0f;
            static float zAngle = 0.0f;
            auto camPos = cam.getPosition();
            auto camDir = cam.getDirection();
            if (mouse->getScrollDelta())
            {
                Debug::debug(mouse->getScrollDelta());
            }

            if (keyboard->isKeyPressed(VK_ESCAPE))
            {
                Debug::debug("click f10");
            }
            if (!isEditorMode)
            {
                if (keyboard->isKeyHeld(0x53))
                {
                    cam.moveTo(camPos + camDir * 0.1f);
                }
                if (keyboard->isKeyHeld(0x57))
                {
                    cam.moveTo(camPos - camDir * 0.1f);
                }
                if (keyboard->isKeyHeld(0x41))
                {
                    auto rightVec = camDir.cross({0.0f, 1.0f, 0.0f});
                    rightVec.normalize();
                    cam.moveTo(camPos + rightVec * 0.1f);
                }
                if (keyboard->isKeyHeld(0x44))
                {
                    auto rightVec = camDir.cross({0.0f, 1.0f, 0.0f});
                    rightVec.normalize();
                    cam.moveTo(camPos - rightVec * 0.1f);
                }
            }
            else
            {
                if (keyboard->isKeyPressed(0x31))
                {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                }
                if (keyboard->isKeyPressed(0x32))
                {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
                }
                if (keyboard->isKeyPressed(0x33))
                {
                    // temp
                    // nb::OpenGl::OpenGLRender::setIsOrtho(true);
                    // auto scene = renderer->getScene();
                    // scene.pop_back();
                    // for (auto &i : vert)
                    // {
                    //     i.color = nb::Math::toFloatColor(Math::Vector3<uint8_t>(0 + rand() % 255, 0 + rand() % 255, 0 + rand() % 255));
                    // }
                    // scene.push_back(createRef<Renderer::Mesh>(vert, ind));
                    // renderer->setScenes(scene);
                    nb::OpenGl::OpenGLRender::applyDefaultModel();
                }
                if (keyboard->isKeyPressed(0x34))
                {
                    // auto scene = renderer->getScene();
                    // scene.pop_back();
                    // for (auto &i : vert)
                    // {
                    //     i.color = nb::Math::toFloatColor(Math::Vector3<uint8_t>(0 + rand() % 255, 0 + rand() % 255, 0 + rand() % 255));
                    // }
                    // scene.push_back(createRef<Renderer::Mesh>(vert, ind));
                    // renderer->setScenes(scene);
                    nb::OpenGl::OpenGLRender::applyDefaultModelFlat();
                    // temp
                    // nb::OpenGl::OpenGLRender::setIsOrtho(false);
                }
                if (keyboard->isKeyPressed(0x35))
                {
                    nb::OpenGl::OpenGLRender::rotate(15.0f, {1.0f, 0.0f, 0.0f});
                }
                if (keyboard->isKeyPressed(0x36))
                {
                    nb::OpenGl::OpenGLRender::rotate(15.0f, {0.0f, 1.0f, 0.0f});
                }
                if (keyboard->isKeyPressed(0x37))
                {
                    nb::OpenGl::OpenGLRender::rotate(15.0f, {0.0f, 0.0f, 1.0f});
                }
                if (keyboard->isKeyPressed(0x38))
                {
                    nb::OpenGl::OpenGLRender::setIsOrtho(true);
                    cam.moveTo({0.0f, 2.0f, 4.0f});
                    nb::OpenGl::OpenGLRender::setModel();
                }
                if(keyboard->isKeyPressed(VK_SHIFT))
                {
                    nb::OpenGl::OpenGLRender::enableLerpMaterial(true);
                }
                if(keyboard->isKeyPressed(0x39))
                {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
            }

            if (keyboard->isKeyPressed(VK_ESCAPE))
            {
                isEditorMode = !isEditorMode;
                if (isEditorMode)
                    while (ShowCursor(true) < 0)
                        ;
                else
                    while (ShowCursor(false) > 0)
                        ;
            }

            if (keyboard->isKeyPressed(VK_F10))
            {
                if (isSampling)
                {
                    glDisable(GL_MULTISAMPLE);
                    isSampling = false;
                }
                else
                {
                    glEnable(GL_MULTISAMPLE);
                    isSampling = true;
                }
            }

            renderer->render();
            return true;
        }

        void Engine::rayPick(const uint32_t x, const uint32_t y) noexcept
        {
            // Debug::debug(x);
            // Debug::debug(y);
            //             Debug::debug("-----------");
            //                 Debug::debug(EngineSettings::getWidth());
            //                 Debug::debug(EngineSettings::getHeight());

            nb::Math::RayPicker rp;
            auto a = rp.cast(renderer->getCamera(), x, y, EngineSettings::getWidth(), EngineSettings::getHeight());
            auto scene = renderer->getScene()->getScene();
            

            std::stack<std::shared_ptr<Renderer::BaseNode>> stk;
            stk.push(scene);

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
                    // auto aabb = n->mesh->getAABB();

                    // auto camPos = renderer->getCamera()->getPosition();
                    // Ray ray;
                    // ray.direction = a;
                    // ray.origin = camPos;

                    // if (rayIntersectsAABB(ray, aabb.minPoint, aabb.maxPoint))
                    // {
                    //     OpenGl::OpenGLRender::setAmbientLight({255, 120, 100});
                    //     //if (mouse->isLeftHeld())
                    //     {
                    //         // auto p = OpenGl::OpenGLRender::getAdd();
                    //         // OpenGl::OpenGLRender::drawTransformationElements(i);
                    //         //Debug::debug("Inter");
                    //     }
                    // }
                    // else
                    // {
                    //     OpenGl::OpenGLRender::setAmbientLight({0, 0, 0});
                    // }

                    // auto xx = std::to_string(a.x) + "---" + std::to_string(cam.getDirection().x);
                    // auto yy = std::to_string(a.y) + "---" + std::to_string(cam.getDirection().y);
                    // auto zz = std::to_string(a.z) + "---" + std::to_string(cam.getDirection().z);

                    // Debug::debug(xx.c_str());
                    // Debug::debug(yy.c_str());
                    // Debug::debug(zz.c_str());
                }
                
            }


            // for (const auto &i : scene)
            // {
            //     auto aabb = i->getAABB();

            //     auto camPos = renderer->getCamera()->getPosition();
            //     Ray ray;
            //     ray.direction = a;
            //     ray.origin = camPos;

            //     // Plane l;
                // l.point = i->getAABB().center();
                // l.normal = Math::Vector3<float>(0.0f, 0.0f, 1.0f);

                // nb::Math::Vector3<float> delta;
                // nb::Math::Vector2<float> curs{static_cast<float>(x), static_cast<float>(y)};
                // nb::Math::Vector2<int> wh{EngineSettings::getWidth(), EngineSettings::getHeight()};

                // if (intersect(ray, l, l.point, l.point + nb::Math::Vector3<float>{10.f,0.0f,0.0f},
                // renderer->getCamera()->getProjection(),renderer->getCamera()->getLookAt(),
                // wh,curs,5.0f,delta))
                // {
                //     OpenGl::OpenGLRender::setAmbientLight({115, 120, 100});
                //     Debug::debug(delta.x);
                //     Debug::debug(delta.y);
                //     Debug::debug(delta.z);
                //     Debug::debug("inter");
                //     return;
                // }

                // if (rayIntersectsAABB(ray, aabb.minPoint, aabb.maxPoint))
                // {
                //     OpenGl::OpenGLRender::setAmbientLight({255, 120, 100});
                //     if (mouse->isLeftHeld())
                //     {
                //         // auto p = OpenGl::OpenGLRender::getAdd();
                //         // OpenGl::OpenGLRender::drawTransformationElements(i);
                //     }
                // }
                // else
                // {
                //     //OpenGl::OpenGLRender::setAmbientLight({0, 0, 0});
                // }
            }
            //Math::Vector3<float> intersection = {camPos.x + i * a.x,
             //                camPos.y + i * a.y,
             //                camPos.z + i * a.z};
           
        
        HWND Engine::hwnd = nullptr;
    };
};
