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
            static nb::Renderer::Camera cam;
            renderer->setCamera(&cam);

            static float yaw;
            static float pitch;
            
            if(!isEditorMode)
                cam.update(mouse->getYaw(), mouse->getPitch());

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
                
                //if(isEditorMode)
               //if(msg.message == WM_INPUT)
                    input->update(msg);

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

            if(keyboard->isKeyPressed(VK_ESCAPE))
            {
                isEditorMode = !isEditorMode;
                if(isEditorMode) while (ShowCursor(true) < 0);
                else while(ShowCursor(false) > 0);
            }
            


            renderer->render();
            return true;
        }

        HWND Engine::hwnd = nullptr;
    };
};
