
#include <windows.h>
#include <../../Engine/src/Core/Engine.hpp>
#include <../../Engine/src/Math/Quaternion.hpp>

#include <../../Engine/src/Core/EngineSettings.hpp>

#include "SceneWindow.hpp"
#include "HierarchyWindow.hpp"
#include "PropertiesWindow.hpp"

HWND hMDIClient = nullptr;
HMENU hMainMenu = nullptr;

HWND activeWindow = nullptr;

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MDIChildWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void RegisterMDIChildClass(HINSTANCE hInstance);
HWND hChild;
std::shared_ptr<nb::Core::Engine> engine = nullptr;
HWND hwndMain;

Editor::SceneWindow *scene;
Editor::HierarchyWindow *hierarchyWindow;
Editor::PropertiesWindow *propertiesWindow;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    const wchar_t *mainClassName = L"MDIMainWindow";
    AllocConsole();
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_TREEVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    WNDCLASS wc = {};
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = mainClassName;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    RegisterMDIChildClass(hInstance);

    hwndMain = CreateWindowEx(
        0, mainClassName, L"SDK",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1000, 1000,
        nullptr, nullptr, hInstance, nullptr);

    if (!hwndMain)
    {
        MessageBox(nullptr, L"Failed to create main window!", L"Error", MB_ICONERROR);
        return -1;
    }

    ShowWindow(hwndMain, nCmdShow);
    UpdateWindow(hwndMain);

    MDICREATESTRUCT mcs = {};

    mcs.x = 0;
    mcs.y = 0;
    mcs.cx = 437;
    mcs.cy = 295;
    mcs.szTitle = L"Child Window";
    mcs.szClass = L"MDIChildClass";
    mcs.hOwner = GetModuleHandle(nullptr);
    mcs.style = WS_OVERLAPPEDWINDOW;
    RECT rect;
    GetClientRect(hMDIClient, &rect);

    scene = new Editor::SceneWindow(hMDIClient, engine);
    hChild = scene->getHandle();
    engine = std::make_shared<nb::Core::Engine>(hChild);
    scene->setEngine(engine);
    scene->resize(rect.right - rect.left, rect.bottom - rect.top);

    hierarchyWindow = new Editor::HierarchyWindow(hMDIClient, engine);
    hierarchyWindow->resize(rect.right - rect.left, rect.bottom - rect.top);
    auto s = engine->getScene();
    hierarchyWindow->update();

    propertiesWindow = new Editor::PropertiesWindow(hMDIClient, engine);
    propertiesWindow->resize(rect.right - rect.left, rect.bottom - rect.top);

    MSG msg;

    bool running = true;
    while (running)
    {
        // don't pick 
        // processInput should be first to update state from prev frame 
        engine->processInput();

        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                running = false;
                break;
            }

            if (msg.message == WM_INPUT)
            {
                engine->bufferizeInput(msg);
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!running)
            break;

        engine->run(scene->peekMouseDelta(), scene->getMouseButtons());
    }
    auto i = GetLastError();

    return (int)msg.wParam;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        hMainMenu = CreateMenu();
        HMENU hFileMenu = CreateMenu();
        AppendMenu(hFileMenu, MF_STRING, 1, L"New");
        AppendMenu(hFileMenu, MF_STRING, 2, L"Exit");
        AppendMenu(hMainMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"File");
        SetMenu(hwnd, hMainMenu);

        CLIENTCREATESTRUCT ccs = {};
        ccs.hWindowMenu = hFileMenu;
        ccs.idFirstChild = 50000;

        hMDIClient = CreateWindowEx(
            0, L"MDICLIENT", nullptr,
            WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL,
            0, 0, 0, 0, hwnd, (HMENU)1, GetModuleHandle(nullptr), &ccs);

        if (!hMDIClient)
        {
            MessageBox(hwnd, L"Failed to create MDI client area!", L"Error", MB_ICONERROR);
            PostQuitMessage(-1);
        }

        ShowWindow(hMDIClient, SW_SHOW);
        return 0;
    }

    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case 1:
        {
            MDICREATESTRUCT mcs = {};
            mcs.szTitle = L"Child Window";
            mcs.szClass = L"MDIChildClass";
            mcs.hOwner = GetModuleHandle(nullptr);
            mcs.style = WS_OVERLAPPEDWINDOW;

            HWND Child = (HWND)SendMessage(hMDIClient, WM_MDICREATE, 0, (LPARAM)&mcs);
            if (!Child)
            {
                DWORD error = GetLastError();
                wchar_t errorMsg[256];
                MessageBox(hwnd, errorMsg, L"Error", MB_ICONERROR);
            }
            return 0;
        }

        case 2:
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            return 0;
        }
        break;
    }
    case WM_TIMER:

        break;

    case WM_DRAWITEM:
    {
        DRAWITEMSTRUCT *dis = reinterpret_cast<DRAWITEMSTRUCT *>(lParam);
        if (dis->CtlType == ODT_LISTBOX)
        {
            RECT rect = dis->rcItem;
            HDC hdc = dis->hDC;

            if (dis->itemState & ODS_SELECTED)
            {
                SetBkColor(hdc, RGB(0, 120, 215));
                SetTextColor(hdc, RGB(255, 255, 255));
            }
            else
            {
                SetBkColor(hdc, RGB(255, 255, 255));
                SetTextColor(hdc, RGB(0, 0, 0));
            }

            FillRect(hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));

            int index = dis->itemID;
            wchar_t buffer[256];
            SendMessage(dis->hwndItem, LB_GETTEXT, index, (LPARAM)buffer);
            DrawText(hdc, buffer, -1, &rect, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
        }
        return TRUE;
        break;
    }
    case WM_SIZE:
        if (hMDIClient)
            MoveWindow(hMDIClient, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);

        if (scene != nullptr)
            scene->resize(LOWORD(lParam), HIWORD(lParam));

        if (hierarchyWindow != nullptr)
            hierarchyWindow->resize(LOWORD(lParam), HIWORD(lParam));

        if (propertiesWindow != nullptr)
            propertiesWindow->resize(LOWORD(lParam), HIWORD(lParam));

        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK MDIChildWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_MDIACTIVATE:
    {
        if ((HWND)lParam == hChild && engine != nullptr)
        {
            engine->setHandleInput(true);
        }
        else if ((HWND)wParam == hChild && engine != nullptr)
        {
            engine->setHandleInput(false);
        }

        activeWindow = (HWND)lParam;
        return 0;
    }
    case WM_SIZE:
    {

        break;
    }
    case WM_TIMER:
        break;
    case WM_CREATE:
        return 0;

    case WM_DESTROY:
        return 0;

    default:
        return DefMDIChildProc(hwnd, msg, wParam, lParam);
    }
}

void RegisterMDIChildClass(HINSTANCE hInstance)
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = MDIChildWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"MDIChildClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    if (!RegisterClass(&wc))
    {
        MessageBox(nullptr, L"Failed to register MDI child class!", L"Error", MB_ICONERROR);
    }
}
