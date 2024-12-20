
#include <windows.h>
#include <../../Engine/src/Core/Engine.hpp>
#include <../../Engine/src/Math/Quaternion.hpp>

#include <../../Engine/src/Core/EngineSettings.hpp>

#include "SceneWindow.hpp"
#include "HierarchyWindow.hpp"
#include "PropertiesWindow.hpp"

// Global variables for MDI
HWND hMDIClient = nullptr; // Handle to the MDI client area
HMENU hMainMenu = nullptr; // Handle to the main menu

HWND activeWindow = nullptr;

// Function declarations
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MDIChildWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void RegisterMDIChildClass(HINSTANCE hInstance);
HWND hChild;
std::shared_ptr<nb::Core::Engine> engine = nullptr;
HWND hwndMain;

Editor::SceneWindow *scene;
Editor::HierarchyWindow *hierarchyWindow;
Editor::PropertiesWindow *propertiesWindow;


// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    const wchar_t *mainClassName = L"MDIMainWindow";
    AllocConsole();
    InitCommonControls(); 

    // Register the main window class
    WNDCLASS wc = {};
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = mainClassName;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    // Register the MDI child window class
    RegisterMDIChildClass(hInstance);

    // Create the main (parent) window
    hwndMain = CreateWindowEx(
        0, mainClassName, L"Basic MDI Application",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480,
        nullptr, nullptr, hInstance, nullptr);

    if (!hwndMain)
    {
        MessageBox(nullptr, L"Failed to create main window!", L"Error", MB_ICONERROR);
        return -1;
    }

    // Show and update the main window
    ShowWindow(hwndMain, nCmdShow);
    UpdateWindow(hwndMain);

//
    MDICREATESTRUCT mcs = {};

    mcs.x = 0;
    mcs.y = 0;
    mcs.cx = 437;
    mcs.cy = 295;
    mcs.szTitle = L"Child Window";
    mcs.szClass = L"MDIChildClass";
    mcs.hOwner = GetModuleHandle(nullptr);
    mcs.style = WS_OVERLAPPEDWINDOW;

    scene = new Editor::SceneWindow(hMDIClient, engine);
    hChild = scene->getHandle();
    engine = std::make_shared<nb::Core::Engine>(hChild);
    scene->setEngine(engine);
    
    /// TODO:
    /// resize func incorrect 
    /// > 4 windows
    /// save position after resizing 
    
    RECT rect;
    GetClientRect(hMDIClient, &rect);

    hierarchyWindow = new Editor::HierarchyWindow(hMDIClient, engine);
    hierarchyWindow->resize(rect.right - rect.left, rect.bottom - rect.top);

    propertiesWindow = new Editor::PropertiesWindow(hMDIClient);
    propertiesWindow->resize(rect.right - rect.left, rect.bottom - rect.top);

    //auto q = nb::Math::Quaternion<float>(23, 11, 5, 5);
    //auto nq = q.normalize();
    

    MSG msg;

    while (true)
    {
        if(!PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            continue;

        if (msg.message == WM_QUIT)
            break;
        // if (hMDIClient && TranslateMDISysAccel(hMDIClient, &msg))
        // {
        //     continue;  // Handle MDI accelerators
        // }
        if (msg.hwnd == hChild)
            engine->run(msg);

        TranslateMessage(&msg);
        DispatchMessage(&msg);
        
    }
    auto i = GetLastError();

    return (int)msg.wParam;
}

// Main window procedure
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        // Create the main menu
        hMainMenu = CreateMenu();
        HMENU hFileMenu = CreateMenu();
        AppendMenu(hFileMenu, MF_STRING, 1, L"New");
        AppendMenu(hFileMenu, MF_STRING, 2, L"Exit");
        AppendMenu(hMainMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"File");
        SetMenu(hwnd, hMainMenu);

        // Define the MDI client area structure
        CLIENTCREATESTRUCT ccs = {};
        ccs.hWindowMenu = hFileMenu;
        ccs.idFirstChild = 50000;

        // Create the MDI client area
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
        { // "New" menu item
            // Create a new MDI child window
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
                // swprintf_s(errorMsg, L"Failed to create MDI child window. Error code: %lu", error);
                MessageBox(hwnd, errorMsg, L"Error", MB_ICONERROR);
            }
            return 0;
        }

        case 2: // "Exit" menu item
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            return 0;
        }
        break;
    }
    case WM_TIMER:
    //std::cout << "WM_TIMER received\n";
    // Ensure no unexpected actions occur here
    break;

    // case WM_NCPAINT:
    // {
    //     // Get the device context for the non-client area
    //     HDC hdc = GetWindowDC(hwndMain);
        
    //     // Define the color for the non-client area
    //     COLORREF borderColor = RGB(53,56,57); // Custom border color

    //     // Create a brush for the border color
    //     HBRUSH hBrush = CreateSolidBrush(borderColor);
        
    //     // Get the dimensions of the window
    //     RECT rc;
    //     GetWindowRect(hwndMain, &rc);
    //     OffsetRect(&rc, -rc.left, -rc.top); // Convert to client coordinates
        
    //     // Draw the top border
    //     RECT topBorder = { rc.left, rc.top, rc.right, rc.top + 30 }; // Example size
    //     FillRect(hdc, &topBorder, hBrush);
        
    //     // Draw the left border
    //     RECT leftBorder = { rc.left, rc.top + 30, rc.left + 10, rc.bottom };
    //     FillRect(hdc, &leftBorder, hBrush);
        
    //     // Draw the right border
    //     RECT rightBorder = { rc.right - 10, rc.top + 30, rc.right, rc.bottom };
    //     FillRect(hdc, &rightBorder, hBrush);

    //     RECT botBorder; //= { rc.left, rc.top, rc.right, rc.bottom-30 }; // Example size
    //     botBorder.bottom = rc.bottom;
    //     botBorder.left = rc.left;
    //     botBorder.right = rc.right;
    //     botBorder.top = rc.bottom - 10;
    //     FillRect(hdc, &botBorder, hBrush);
        
    //     // Clean up
    //     DeleteObject(hBrush);
    //     ReleaseDC(hwndMain, hdc);
    //     return 0; // Message handled
    // }



    case WM_DRAWITEM:
    {
        DRAWITEMSTRUCT* dis = reinterpret_cast<DRAWITEMSTRUCT*>(lParam);
        if (dis->CtlType == ODT_LISTBOX) {
            // Draw the item based on its state
            RECT rect = dis->rcItem;
            HDC hdc = dis->hDC;
            
            // Background color
            if (dis->itemState & ODS_SELECTED) {
                SetBkColor(hdc, RGB(0, 120, 215)); // Selected color
                SetTextColor(hdc, RGB(255, 255, 255)); // Text color
            } else {
                SetBkColor(hdc, RGB(255, 255, 255)); // Default background
                SetTextColor(hdc, RGB(0, 0, 0)); // Default text color
            }
            
            // Fill the background
            FillRect(hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));
            
            // Draw the text
            int index = dis->itemID;
            wchar_t buffer[256];
            SendMessage(dis->hwndItem, LB_GETTEXT, index, (LPARAM)buffer);
            DrawText(hdc, buffer, -1, &rect, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
        }
        return TRUE; // Indicate we handled the message

        break;
    }
    case WM_SIZE:
        // Resize the MDI client area to fill the parent window
        if (hMDIClient)
            MoveWindow(hMDIClient, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);

        if(scene != nullptr)
            scene->resize(LOWORD(lParam), HIWORD(lParam));

        if(hierarchyWindow != nullptr)
            hierarchyWindow->resize(LOWORD(lParam), HIWORD(lParam));

        if(propertiesWindow != nullptr)
            propertiesWindow->resize(LOWORD(lParam), HIWORD(lParam));

        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// MDI child window procedure
LRESULT CALLBACK MDIChildWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_MDIACTIVATE:
    {
        if ((HWND)lParam == hChild && engine != nullptr)
        {
            engine->setHandleInput(true);
            //while (ShowCursor(false) > 0);
        }
        else if ((HWND)wParam == hChild && engine != nullptr)
        {
            engine->setHandleInput(false);
            //while (ShowCursor(true) < 0);
        }

        activeWindow = (HWND)lParam;
        return 0;
    }
    case WM_SIZE:
    {
        // if (activeWindow == hChild)
        // {
        //     nb::Core::EngineSettings::setHeight(HIWORD(lParam));
        //     nb::Core::EngineSettings::setWidth(LOWORD(lParam));
        // }
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

    default:
        return DefMDIChildProc(hwnd, msg, wParam, lParam);
    }
}

// Register the MDI child window class
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
