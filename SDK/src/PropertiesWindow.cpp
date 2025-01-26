#include "PropertiesWindow.hpp"
#include "HierarchyWindow.hpp"

namespace Editor
{
    PropertiesWindow::PropertiesWindow(const HWND &parentHwnd, std::shared_ptr<nb::Core::Engine> engine)
    {
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
        mcs.szTitle = L"PROPERTIES";
        mcs.szClass = CLASS_NAME;
        mcs.hOwner = GetModuleHandle(nullptr);
        mcs.style = WS_OVERLAPPEDWINDOW;

        hwnd = (HWND)SendMessage(parentHwnd, WM_MDICREATE, 0, (LPARAM)&mcs);
        if (!hwnd)
        {
            DWORD error = GetLastError();
            wchar_t errorMsg[256];
            MessageBox(NULL, errorMsg, L"Error", MB_ICONERROR);
        }

        // slider = new Ui::Slider(hwnd, 50, 50, 300, 30, 0, 100, 50, 1);
        // rAmbient = new Ui::Slider(hwnd, 50, 100, 300, 30, 0, 255, 0, 2);
        // gAmbient = new Ui::Slider(hwnd, 50, 150, 300, 30, 0, 255, 0, 3);
        // bAmbient = new Ui::Slider(hwnd, 50, 200, 300, 30, 0, 255, 0, 4);

        // xPos = new Ui::Slider(hwnd, 50, 250, 300, 30, -10, 10, 0, 5);
        // yPos = new Ui::Slider(hwnd, 50, 300, 300, 30, -10, 10, 0, 6);
        // zPos = new Ui::Slider(hwnd, 50, 350, 300, 30, -10, 10, 0, 7);

        lightModel = new Ui::ComboBox(hwnd, GetModuleHandle(nullptr));
        lightModel->create(500, 500, 200, 200);
        lightModel->addItem(L"Default");
        lightModel->addItem(L"Diffuse Reflection");

        sEngine = engine;

        CreateWindowEx(
                0,                              // extended style
                L"STATIC",                        // class name
                L"Translate",                 // text for the label
                WS_CHILD | WS_VISIBLE,           // styles
                10, 10,                          // x and y position
                100, 30,                         // width and height
                hwnd,                            // parent window
                (HMENU) 1,                       // ID
                (HINSTANCE) GetWindowLong(hwnd, GWLP_HINSTANCE), // instance handle
                NULL                             // additional data
            );

        CreateWindowEx(
                        0,                              // extended style
                        L"STATIC",                        // class name
                        L"Rotate",                 // text for the label
                        WS_CHILD | WS_VISIBLE,           // styles
                        160, 10,                          // x and y position
                        100, 30,                         // width and height
                        hwnd,                            // parent window
                        (HMENU) 2,                       // ID
                        (HINSTANCE) GetWindowLong(hwnd, GWLP_HINSTANCE), // instance handle
                        NULL                             // additional data
                    );

        CreateWindowEx(
                        0,                              // extended style
                        L"STATIC",                        // class name
                        L"Scale",                 // text for the label
                        WS_CHILD | WS_VISIBLE,           // styles
                        310, 10,                          // x and y position
                        100, 30,                         // width and height
                        hwnd,                            // parent window
                        (HMENU) 3,                       // ID
                        (HINSTANCE) GetWindowLong(hwnd, GWLP_HINSTANCE), // instance handle
                        NULL                             // additional data
                    );


        coordsEdit = new CoordinateEditor(hwnd, 0, 0, 0);
        rotateEdit = new CoordinateEditor(hwnd, 0, 0, 0, 150);
        scaleEdit = new CoordinateEditor(hwnd, 0, 0, 0, 300);

        radioShadingModel = new Ui::RadioGroup(hwnd, L"Lights Model", {
            {L"Default", []() { nb::OpenGl::OpenGLRender::applyDefaultModel(); }},
            {L"Diffuse Reflection", []() { nb::OpenGl::OpenGLRender::applyDiffuseReflectionModel(); }}
        });

        // LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
        // style &= ~WS_CAPTION;
        // SetWindowLongPtr(hwnd, GWL_STYLE, style);
    }

    PropertiesWindow::~PropertiesWindow()
    {
        delete rAmbient;
        delete gAmbient;
        delete bAmbient;
        delete slider;
    }

    void PropertiesWindow::resize(const int newWidth, const int newHeigth) noexcept
    {
        const int width = newWidth * WIDTH_PROPORTION;
        const int height = newHeigth * HEIGHT_PROPORTION;

        // Debug::debug(width);
        // Debug::debug(height);

        MoveWindow(hwnd, newWidth - width, newHeigth - height, width, height, TRUE);
    }

    LRESULT PropertiesWindow::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if(radioShadingModel != nullptr)
            radioShadingModel->handle(message, wParam, lParam);

        switch (message)
        {
        case WM_CREATE:
            return 0;
       
        // case WM_NOTIFY:
        // {
        //     auto currentObj = sEngine->getScene()->find(Editor::HierarchyWindow::getSelectedObjectName());
        //     auto currentTransform = currentObj->getLocalTransform();
        //     auto nCode = ((LPNMHDR)lParam)->code;

        //     switch (nCode)
        //     {
        //     case UDN_DELTAPOS:
            
        //         auto lpnmud = (LPNMUPDOWN)lParam;
        //         if(lpnmud->hdr.hwndFrom == coordsEdit->GetSpinXHandle()
        //             || lpnmud->hdr.hwndFrom == coordsEdit->GetSpinYHandle()
        //             || lpnmud->hdr.hwndFrom == coordsEdit->GetSpinZHandle())
        //         {
        //             currentObj->setTranslate({coordsEdit->getX(), coordsEdit->getY(), coordsEdit->getZ()});
        //         }

        //         break;
        //     }
        // }
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
            //     // nb::OpenGl::OpenGLRender::setAmbientLight(nb::Math::Vector3<uint8_t>(rAmbient->GetValue(), gAmbient->GetValue(), bAmbient->GetValue())); // nb::Core::EngineSettings::setFov(value);
            //     if (currentObj != nullptr)
            //         currentObj->setTransform(nb::Math::translate(currentObj->getLocalTransform(), {(float)value, 0.0f, 0.0f}));
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

            // if ((HWND)lParam == xPos->GetHandle())
            // {
            //     int value = xPos->GetValue();
            //     nb::OpenGl::OpenGLRender::setLightPos({float(value), pos.y, pos.z});
            // }
            // if ((HWND)lParam == yPos->GetHandle())
            // {
            //     int value = yPos->GetValue();
            //     nb::OpenGl::OpenGLRender::setLightPos({pos.x, (float)value, pos.z});
            // }
            // if ((HWND)lParam == zPos->GetHandle())
            // {
            //     int value = zPos->GetValue();
            //     nb::OpenGl::OpenGLRender::setLightPos({pos.x, pos.y, (float)value});
            // }
            return 0;
        }

        case WM_SIZE:
        {
            // Debug::debug("RESIZE");
            break;
        }
        case WM_COMMAND:
        {
            if (lightModel && (HWND)lParam == lightModel->getHandle())
            {
                if (HIWORD(wParam) == CBN_SELCHANGE)
                {
                    int index = lightModel->getSelectedIndex();
                    switch (index)
                    {
                    case 0:
                    {
                        nb::OpenGl::OpenGLRender::applyDefaultModel();
                        break;
                    }
                    case 1:
                    {
                        nb::OpenGl::OpenGLRender::applyDiffuseReflectionModel();
                        break;
                    }

                    default:
                        break;
                    }
                }
            }

            if (HIWORD(wParam) == EN_CHANGE) 
            {
                if(!currentObject)
                    return 0;

               //auto currentTransform = currentObject->getLocalTransform();
                //coordsEdit->setCoordinates(currentObj->getTransform().translate);

                currentObject->setTranslate({coordsEdit->getX(), coordsEdit->getY(), coordsEdit->getZ()});
                currentObject->setRotationX(rotateEdit->getX());
                currentObject->setRotationY(rotateEdit->getY());
                currentObject->setRotationZ(rotateEdit->getZ());
                currentObject->setScale({scaleEdit->getX(), scaleEdit->getY(), scaleEdit->getZ()});
            }

            return 0;
        }

        case WM_DESTROY:
            return 0;

        default:
            return DefMDIChildProc(hwnd, message, wParam, lParam);
        }
    }

    void PropertiesWindow::setCurrentObject(std::shared_ptr<nb::Renderer::BaseNode> obj) noexcept
    {
        currentObject = obj;
        auto transform = obj->getTransform();
        coordsEdit->setCoordinates(transform.translate);
        rotateEdit->setCoordinates({transform.rotateX, transform.rotateY, transform.rotateZ});
        scaleEdit->setCoordinates(transform.scale);
    }

    Ui::Slider *PropertiesWindow::slider = nullptr;
    Ui::Slider *PropertiesWindow::rAmbient = nullptr;
    Ui::Slider *PropertiesWindow::gAmbient = nullptr;
    Ui::Slider *PropertiesWindow::bAmbient = nullptr;

    Ui::Slider *PropertiesWindow::xPos = nullptr;
    Ui::Slider *PropertiesWindow::yPos = nullptr;
    Ui::Slider *PropertiesWindow::zPos = nullptr;

    Ui::ComboBox *PropertiesWindow::lightModel = nullptr;

    nb::Math::Vector3<float> PropertiesWindow::pos = {0.0f, 0.0f, 0.0f};
    std::shared_ptr<nb::Core::Engine> PropertiesWindow::sEngine = nullptr;

    Editor::CoordinateEditor *PropertiesWindow::coordsEdit = nullptr;
    Editor::CoordinateEditor *PropertiesWindow::rotateEdit = nullptr;
    Editor::CoordinateEditor *PropertiesWindow::scaleEdit = nullptr;

    Ui::RadioGroup *PropertiesWindow::radioShadingModel = nullptr;



    std::shared_ptr<nb::Renderer::BaseNode> PropertiesWindow::currentObject = nullptr;




};