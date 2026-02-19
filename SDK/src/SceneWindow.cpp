// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "SceneWindow.hpp"
#include <../../Engine/src/Math/RayCast/RayPicker.hpp>
namespace Editor
{

    SceneWindow::SceneWindow(
        HWND parentHwnd,
        std::shared_ptr<nb::Core::Engine> engine
    )
        :engine(engine)
    {
        Sengine = engine;
        WNDCLASS wc = {};
        wc.lpfnWndProc = WndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = CLASS_NAME;
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        if (!RegisterClass(&wc))
        {
            MessageBox(nullptr, L"Failed to register MDI child class!", L"Error", MB_ICONERROR);
        }

        MDICREATESTRUCT mcs = {};

        mcs.x = 0;
        mcs.y = 0;
        mcs.cx = 0;
        mcs.cy = 0;
        mcs.szTitle = L"Child Window";
        mcs.szClass = CLASS_NAME;
        mcs.hOwner = GetModuleHandle(nullptr);
        mcs.style = WS_CHILD | WS_VISIBLE ;

        hwnd = (HWND)SendMessage(parentHwnd, WM_MDICREATE, 0, (LPARAM)&mcs);
        if (!hwnd)
        {
            DWORD error = GetLastError();
            wchar_t errorMsg[256];
            swprintf_s(errorMsg, L"Failed to create MDI child window. Error code: %lu", error);
            MessageBox(NULL, errorMsg, L"Error", MB_ICONERROR);
        }

        // no resize
        LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
        style &= ~WS_THICKFRAME;
        style &= ~WS_BORDER;
        SetWindowLongPtr(hwnd, GWL_STYLE, style);
        //
    }

    LRESULT SceneWindow::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_MDIACTIVATE:
        {
            // if ((HWND)lParam == hChild && engine != nullptr)
            // {
            //     engine->setHandleInput(true);
            //     //while (ShowCursor(false) > 0);
            // }
            // else if ((HWND)wParam == hChild && engine != nullptr)
            // {
            //     engine->setHandleInput(false);
            //     //while (ShowCursor(true) < 0);
            // }

            // activeWindow = (HWND)lParam;
            // return 0;
        }
//         case WM_MOUSEMOVE:
//         {
//                             if(Sengine != nullptr)
// {
//                 isPrevClick = true;
//                 //Debug::debug(LOWORD(lParam));
//                 //Debug::debug(HIWORD(lParam));

//                 int xw = LOWORD(lParam);
//                 int yw = HIWORD(lParam);

//                 //float xn = float((2.0f * (float)xw) / (float)nb::Core::EngineSettings::getWidth()) - 1.0f;
//                 //float yn = float((2.0f * (float)yw) / (float)nb::Core::EngineSettings::getHeight()) - 1.0f;
//                 //Debug::debug(xn);
//                 //Debug::debug(yn);
//                 if(Sengine != nullptr)
//                     Sengine->rayPick(xw, yw);
// }           
//             break;
//         }
        case WM_MOUSEMOVE:
        {
            if(Sengine != nullptr)
            {
                static int x = 0;
                static int y = 0;

                x = LOWORD(lParam);
                y = HIWORD(lParam);

                mouseDelta.addDelta(x - mouseDelta.dx, y - mouseDelta.dy);

                Sengine->rayPick(x, y);

            }

                
                ///Debug::debug(LOWORD(lParam));
                //Debug::debug(HIWORD(lParam));
        }
        break;
        case WM_LBUTTONUP:
        {
            mouseButtons &= 0x0002;
            isPrevClick = false;
            break;
        }
        case WM_LBUTTONDOWN:
        {
            mouseButtons &= 0x0001;
            break;
        }
        case WM_SIZE:
        {
            Debug::debug(HIWORD(lParam));
            Debug::debug(LOWORD(lParam));

            //nb::Core::EngineSettings::setHeight(HIWORD(lParam));
            //nb::Core::EngineSettings::setWidth(LOWORD(lParam));
            
            break;
        }
        case WM_TIMER:
        // Ensure no unexpected actions occur here
        break;
        case WM_CREATE:
            // SetWindowText(hwnd, L"MDI Child");
            return 0;

        case WM_DESTROY:
            return 0;

        case WM_HSCROLL:
        {

            // if ((HWND)lParam == slider->GetHandle())
            // {
            //     int value = slider->GetValue();
            //     nb::Core::EngineSettings::setFov(value);
            // }

            // if ((HWND)lParam == rAmbient->GetHandle())
            // {
            //     int value = rAmbient->GetValue();
            //     nb::OpenGl::OpenGLRender::setAmbientLight(nb::Math::Vector3<uint8_t>(rAmbient->GetValue(), gAmbient->GetValue(), bAmbient->GetValue())); // nb::Core::EngineSettings::setFov(value);
            // }
            // if ((HWND)lParam == gAmbient->GetHandle())
            // {
            //     int value = slider->GetValue();
            //     nb::OpenGl::OpenGLRender::setAmbientLight(nb::Math::Vector3<uint8_t>(rAmbient->GetValue(), gAmbient->GetValue(), bAmbient->GetValue())); // nb::Core::EngineSettings::setFov(value);
            // }
            // if ((HWND)lParam == bAmbient->GetHandle())
            // {
            //     int value = slider->GetValue();
            //     nb::OpenGl::OpenGLRender::setAmbientLight(nb::Math::Vector3<uint8_t>(rAmbient->GetValue(), gAmbient->GetValue(), bAmbient->GetValue()));
            // }
        }

        default:
            return DefMDIChildProc(hwnd, message, wParam, lParam);
        }

        return DefMDIChildProc(hwnd, message, wParam, lParam);
    }

    nb::Input::MouseDelta SceneWindow::peekMouseDelta() noexcept
    {
        nb::Input::MouseDelta copy = mouseDelta;
        mouseDelta.clear();

        return copy;
    }

    void SceneWindow::resize(const int newWidth, const int newHeigth) noexcept
    {
        const int width = newWidth * WIDTH_PROPORTION;
        const int height = newHeigth * HEIGHT_PROPORTION;

        nb::Core::EngineSettings::setHeight(height);
        nb::Core::EngineSettings::setWidth(width);

        //Debug::debug(width);
        //Debug::debug(height);

        MoveWindow(hwnd, 0, 0, width, height, TRUE);
        RECT r;
        GetClientRect(hwnd, &r);
        nb::Core::EngineSettings::setHeight(r.bottom - r.top);
        nb::Core::EngineSettings::setWidth(r.right - r.left);
    }

    bool SceneWindow::isPrevClick = false;
    std::shared_ptr<nb::Core::Engine> SceneWindow::Sengine = nullptr;
};