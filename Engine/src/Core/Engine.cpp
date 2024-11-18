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
                if(msg.message == WM_QUIT)
                    return false;
                
                if(handleInput)
                    input->update(msg);

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            //}

            // if(mouse->isLeftClick())
            //     Debug::debug("click");

            // if(mouse->isRightHeld())
            //     Debug::debug("held");

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
                auto camPos = cam.getPosition();
                cam.moveTo({camPos.x, camPos.y, camPos.z-0.1f});
            }

            renderer->render();
            return true;
        }

        HWND Engine::hwnd = nullptr;
    };
};
