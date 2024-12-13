#include "Engine.hpp"

namespace nb
{
    namespace Core
    {
        Engine::Engine(const HWND &hwnd)
        {
            subSystems->Init(hwnd);
            this->hwnd = hwnd;
            renderer = subSystems->getRenderer();
            keyboard = subSystems->getKeyboard();
            mouse = subSystems->getMouse();
            input = createRef<Input::Input>();
            input->linkKeyboard(keyboard);
            input->linkMouse(mouse);
        }


        bool Engine::run(const MSG& msg)
        {
          
            //MSG msg;

              // input->update();
            //       keyboard->update();
            //       mouse->update();
            // keyboard->update();
            //static nb::Renderer::Camera cam;
            renderer->setCamera(&cam);

            static float yaw;
            static float pitch;

            if(isEditorMode)
            {
                if(mouse->isLeftClick())
                {
                    Debug::debug("clk");
                }
            }

            
            cam.update(mouse->getX(), mouse->getY());
            if(!isEditorMode)
            {
                input->startHandlingPosition();
                input->startHandlingKeyboardEvents();
            }
            else
            {
                input->stopHandlingPosition();
                //input->stopHandlingKeyboardEvents();
            }

            //if (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE))
            //{
                // if (msg.message == WM_KEYDOWN)
                // {
                //     keyboard->setKeyDown(msg.wParam);
                // }
                // if (msg.message == WM_KEYUP)
                // {
                //     keyboard->setKeyUp(msg.wParam);
                // }
                // if(msg.message == WM_SETFOCUS)
                // {
                //     input->registerInput();
                // }
                // if (msg.message == WM_KILLFOCUS)
                // {
                //     input->unRegisterInput();
                // }

                if(!isEditorMode)
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

                if(msg.message == WM_QUIT)
                    return false;
                
                input->update(msg);
                
               //if(msg.message == WM_INPUT)

                //TranslateMessage(&msg);
                //DispatchMessage(&msg);
            //}

            // if(mouse->isLeftClick())
            //     Debug::debug("click");

            // if(mouse->isRightHeld())
            //     Debug::debug("held");
            auto camPos = cam.getPosition();
            auto camDir = cam.getDirection();
            if(mouse->getScrollDelta())
            {
                Debug::debug(mouse->getScrollDelta());
            }

            if (keyboard->isKeyPressed(VK_ESCAPE))
            {
                Debug::debug("click f10");
            }
            if(!isEditorMode)
            {
                if (keyboard->isKeyHeld(0x53))
                {
                    cam.moveTo(camPos + camDir * 0.1f);
                }
                if(keyboard->isKeyHeld(0x57))
                {
                    cam.moveTo(camPos - camDir * 0.1f);
                }
                if(keyboard->isKeyHeld(0x41))
                {
                    auto rightVec = camDir.cross({0.0f, 1.0f, 0.0f});
                    rightVec.normalize();
                    cam.moveTo(camPos + rightVec * 0.1f);
                }
                if(keyboard->isKeyHeld(0x44))
                {
                    auto rightVec = camDir.cross({0.0f, 1.0f, 0.0f});
                    rightVec.normalize();
                    cam.moveTo(camPos - rightVec * 0.1f);
                }
            }
            else
            {
                if(keyboard->isKeyHeld(VK_CONTROL) && keyboard->isKeyPressed(0x5A))
                {
                    cam.toggleAlignByZ();
                }
                if(keyboard->isKeyHeld(VK_CONTROL) && keyboard->isKeyPressed(0x59))
                {
                    cam.toggleAlignByY();
                }
                if(keyboard->isKeyHeld(VK_CONTROL) && keyboard->isKeyPressed(0x58))
                {
                    cam.toggleAlignByX();
                }
            }


            if(keyboard->isKeyPressed(VK_ESCAPE))
            {
                isEditorMode = !isEditorMode;
                if(isEditorMode) while (ShowCursor(true) < 0);
                else while(ShowCursor(false) > 0);
            }

            if(keyboard->isKeyPressed(VK_F10))
            {
                if(isSampling)
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
            nb::Math::RayPicker rp;
            auto a = rp.cast(renderer->getCamera(), x, y, EngineSettings::getWidth(), EngineSettings::getHeight());
            Debug::debug(a.x);
            Debug::debug(a.y);
            Debug::debug(a.z);
            Debug::debug("__________");
            OpenGl::OpenGLRender::add(a);
        }

        HWND Engine::hwnd = nullptr;
    };
};
