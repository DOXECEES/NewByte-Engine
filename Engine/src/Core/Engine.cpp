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

        void Engine::run()
        {
            MSG msg;

            // input->update();
            //       keyboard->update();
            //       mouse->update();
            // keyboard->update();
            static nb::Renderer::Camera cam;
            renderer->setCamera(&cam);
            cam.update(mouse->getYaw(), mouse->getPitch());

            if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
            {
                // if (msg.message == WM_KEYDOWN)
                // {
                //     keyboard->setKeyDown(msg.wParam);
                // }
                // if (msg.message == WM_KEYUP)
                // {
                //     keyboard->setKeyUp(msg.wParam);
                // }

               
                input->update(msg);

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

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

            if (keyboard->isKeyPressed(0x39))
            {
                auto camPos = cam.getPosition();
                cam.moveTo({camPos.x + 0.1f, camPos.y, camPos.z});
            }

            renderer->render();
        }

        HWND Engine::hwnd = nullptr;
    };
};
