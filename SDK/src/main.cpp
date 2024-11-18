

// #include <windows.h>
// #include <../../Engine/src/Core/Engine.hpp> // Путь к вашему движку
// #include "SceneWindow.hpp"
// #include "UiStore.hpp"
// #include <vector>

// LRESULT CALLBACK HelloWndProc(HWND, UINT, WPARAM, LPARAM);
// LRESULT CALLBACK ChildProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

// HWND childhwnd;
// HWND hWnd;

// int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
//                    PSTR szCMLine, int iCmdShow)
// {
//     static TCHAR szAppName[] = TEXT("HelloApplication");
//     MSG msg;
//     WNDCLASS wndclass;

//     wndclass.style = CS_HREDRAW | CS_VREDRAW;
//     wndclass.lpfnWndProc = HelloWndProc;
//     wndclass.cbClsExtra = 0;
//     wndclass.cbWndExtra = 0;
//     wndclass.hInstance = hInstance;
//     wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
//     wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
//     wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
//     wndclass.lpszMenuName = NULL;
//     wndclass.lpszClassName = szAppName;

//     if (!RegisterClass(&wndclass))
//     {
//         MessageBox(NULL, TEXT("This program requires Windows 95/98/NT"),
//                    szAppName, MB_ICONERROR);
//         return 0;
//     }

//     //  WNDCLASS wndclassC;

//     // wndclassC.style = CS_HREDRAW | CS_VREDRAW;
//     // wndclassC.lpfnWndProc = ChildProc;
//     // wndclassC.cbClsExtra = 0;
//     // wndclassC.cbWndExtra = 0;
//     // wndclassC.hInstance = hInstance;
//     // wndclassC.hIcon = LoadIcon(NULL, IDI_APPLICATION);
//     // wndclassC.hCursor = LoadCursor(NULL, IDC_ARROW);
//     // wndclassC.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
//     // wndclassC.lpszMenuName = NULL;
//     // wndclassC.lpszClassName = L"CHILDR";

//     // if (!RegisterClass(&wndclassC))
//     // {
//     //     MessageBox(NULL, TEXT("This program requires Windows 95/98/NT"),
//     //                szAppName, MB_ICONERROR);
//     //     return 0;
//     // }

//     hWnd = CreateWindow(szAppName,
//                         TEXT("Hello World for Windows"),
//                         WS_OVERLAPPEDWINDOW,
//                         CW_USEDEFAULT,
//                         CW_USEDEFAULT,
//                         CW_USEDEFAULT,
//                         CW_USEDEFAULT,
//                         NULL,
//                         NULL,
//                         hInstance,
//                         NULL);

//     ShowWindow(hWnd, iCmdShow);
//     UpdateWindow(hWnd);

//     // childhwnd = CreateWindow(L"CHILDR",
//     //                          TEXT("Hello World for Windows"),
//     //                          WS_SIZEBOX	 | WS_CHILD,
//     //                          CW_USEDEFAULT,
//     //                          CW_USEDEFAULT,
//     //                          CW_USEDEFAULT,
//     //                          CW_USEDEFAULT,
//     //                          hWnd,
//     //                          NULL,
//     //                          hInstance,
//     //                          NULL);

//     RECT parentRect;
//     GetClientRect(hWnd, &parentRect);

//     auto c = Editor::SceneWindow(hWnd);
//     auto b = Editor::SceneWindow(hWnd);

//     childhwnd = c.getHandle();
//     auto bWnd = b.getHandle();

//     Editor::UiStore::init(parentRect,3,2);
//     Editor::UiStore::add(childhwnd,0,1);
//     Editor::UiStore::add(bWnd, 0, 2);
//     auto e = new nb::Core::Engine(childhwnd);

//     ShowWindow(childhwnd, iCmdShow);
//     UpdateWindow(childhwnd);

//     ShowWindow(bWnd, iCmdShow);
//     UpdateWindow(bWnd);

//     bool isR = true;

//     while (isR)
//     {

//         if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
//         {
//             if (msg.message == WM_QUIT)
//                 isR = false;

//             if (msg.hwnd == childhwnd)
//                 e->run(msg);

//             if (msg.hwnd != childhwnd)
//             {
//                 TranslateMessage(&msg);
//                 DispatchMessage(&msg);
//             }
//         }
//     }
// }

// LRESULT CALLBACK HelloWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
// {
//     HDC hdc;
//     PAINTSTRUCT ps;
//     RECT rect;

//     switch (message)
//     {
//     case WM_CREATE:
//     {
//         RECT parentRect;
//         GetClientRect(hwnd, &parentRect);

//         int parentWidth = parentRect.right - parentRect.left;
//         int parentHeight = parentRect.bottom - parentRect.top;

//         // Позиционируем дочернее окно в правом верхнем углу
//         SetWindowPos(childhwnd, HWND_TOP, 200, 0, 200, 100, 0);
//         ShowWindow(childhwnd, 0);
//         UpdateWindow(childhwnd);
//     }
//     case WM_SIZE:
//     {
//         RECT parentRect;
//         GetClientRect(hwnd, &parentRect);
//         Editor::UiStore::update(parentRect);
//     }
//     case WM_MDIACTIVATE: {
//             // wParam - хэндл активного окна
//             // lParam - хэндл деактивированного окна
//             HWND hwndActive = (HWND)wParam;  // Активное окно
//             HWND hwndDeactivated = (HWND)lParam;  // Деактивированное окно

//             // Обновление информации о текущем активном окне
//             //activeChildWnd = hwndActive;

//             // Пример изменения заголовка родительского окна, когда окно активируется
//             wchar_t title[256];
//             GetWindowText(hwndActive, title, sizeof(title) / sizeof(title[0]));
//             SetWindowText(hwnd, title);  // Устанавливаем в заголовок родительского окна имя активного дочернего окна

//             break;
//         }

//     case WM_PAINT:
//         hdc = BeginPaint(hwnd, &ps);

//         GetClientRect(hwnd, &rect);

//         DrawText(hdc, TEXT("Hello, Windows"), -1, &rect,
//                  DT_SINGLELINE | DT_CENTER | DT_VCENTER);

//         EndPaint(hwnd, &ps);
//         return 0;

//     case WM_DESTROY:
//         PostQuitMessage(0);
//         return 0;
//     }

//     return DefWindowProc(hwnd, message, wParam, lParam);
// }

// LRESULT CALLBACK ChildProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
// {
//     HDC hdc;
//     PAINTSTRUCT ps;
//     RECT rect;

//     switch (message)
//     {
//         case WM_SIZE:
//         {
//             //GetClientRect(childhwnd, &rect);

//             //SetWindowPos(childhwnd, HWND_TOP, 100, 200, LOWORD(lParam), HIWORD(lParam), 0);
//             //return 0;
//         }
//     }
//     return DefWindowProc(hwnd, message, wParam, lParam);

// }

#include <windows.h>
#include <../../Engine/src/Core/Engine.hpp> // Путь к вашему движку
#include <../../Engine/src/Core/EngineSettings.hpp>
// Global variables for MDI
HWND hMDIClient = nullptr; // Handle to the MDI client area
HMENU hMainMenu = nullptr; // Handle to the main menu

HWND activeWindow = nullptr;

// Function declarations
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MDIChildWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void RegisterMDIChildClass(HINSTANCE hInstance);
HWND hChild;
nb::Core::Engine *engine;

#include <commctrl.h> // Для слайдера

#pragma comment(lib, "Comctl32.lib") // Подключение библиотеки для CommCtrl

class Slider
{
public:
    Slider(HWND parent, int x, int y, int width, int height, int minVal, int maxVal, int initVal, int id)
        : hParent(parent), sliderId(id), minValue(minVal), maxValue(maxVal), value(initVal)
    {
        // Создание слайдера
        hSlider = CreateWindowEx(
            0, TRACKBAR_CLASS, nullptr,
            WS_CHILD | WS_VISIBLE | TBS_HORZ,
            x, y, width, height,
            parent, (HMENU)id, GetModuleHandle(nullptr), nullptr);

        if (!hSlider)
        {
            MessageBox(parent, L"Failed to create slider!", L"Error", MB_ICONERROR);
            return;
        }

        // Настройка диапазона и начального значения
        SendMessage(hSlider, TBM_SETRANGE, TRUE, MAKELPARAM(minVal, maxVal));
        SendMessage(hSlider, TBM_SETPOS, TRUE, initVal);
    }

    ~Slider()
    {
        if (hSlider)
            DestroyWindow(hSlider);
    }

    // Получить текущее значение
    int GetValue() const
    {
        return (int)SendMessage(hSlider, TBM_GETPOS, 0, 0);
    }

    // Установить значение
    void SetValue(int val)
    {
        if (val >= minValue && val <= maxValue)
        {
            SendMessage(hSlider, TBM_SETPOS, TRUE, val);
        }
    }

    // Установить диапазон
    void SetRange(int minVal, int maxVal)
    {
        minValue = minVal;
        maxValue = maxVal;
        SendMessage(hSlider, TBM_SETRANGE, TRUE, MAKELPARAM(minVal, maxVal));
    }

    // Получить идентификатор
    int GetId() const
    {
        return sliderId;
    }

    // Получить хендл окна слайдера
    HWND GetHandle() const
    {
        return hSlider;
    }

private:
    HWND hSlider = nullptr;
    HWND hParent = nullptr;
    int sliderId;
    int minValue;
    int maxValue;
    int value;
};

HWND contrl;
Slider *slider;

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    const wchar_t *mainClassName = L"MDIMainWindow";
    AllocConsole();
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
    HWND hwndMain = CreateWindowEx(
        0, mainClassName, L"Basic MDI Application",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        nullptr, nullptr, hInstance, nullptr);

    if (!hwndMain)
    {
        MessageBox(nullptr, L"Failed to create main window!", L"Error", MB_ICONERROR);
        return -1;
    }

    // Show and update the main window
    ShowWindow(hwndMain, nCmdShow);
    UpdateWindow(hwndMain);

    MDICREATESTRUCT mcs = {};
    mcs.szTitle = L"Child Window";
    mcs.szClass = L"MDIChildClass";
    mcs.hOwner = GetModuleHandle(nullptr);
    mcs.style = WS_OVERLAPPEDWINDOW;

    hChild = (HWND)SendMessage(hMDIClient, WM_MDICREATE, 0, (LPARAM)&mcs);
    if (!hChild)
    {
        DWORD error = GetLastError();
        wchar_t errorMsg[256];
        swprintf_s(errorMsg, L"Failed to create MDI child window. Error code: %lu", error);
        MessageBox(hwndMain, errorMsg, L"Error", MB_ICONERROR);
    }
    engine = new nb::Core::Engine(hChild);

    // Main message loop
    contrl = (HWND)SendMessage(hMDIClient, WM_MDICREATE, 0, (LPARAM)&mcs);
    InitCommonControls(); // Инициализация общих элементов управления
    slider = new Slider(contrl, 50, 50, 300, 30, 0, 100, 50, 1);

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {

        if (msg.message == WM_QUIT)
            break;
        // if (hMDIClient && TranslateMDISysAccel(hMDIClient, &msg))
        // {
        //     continue;  // Handle MDI accelerators
        // }
        // if (msg.hwnd == hChild)
        engine->run(msg);

        // if (msg.hwnd != hChild)
        // {
        // TranslateMessage(&msg);
        // DispatchMessage(&msg);
        //}
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

    case WM_SIZE:
        // Resize the MDI client area to fill the parent window
        if (hMDIClient)
            MoveWindow(hMDIClient, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
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
            while (ShowCursor(false) > 0);
        }
        else if ((HWND)wParam == hChild && engine != nullptr)
        {
            engine->setHandleInput(false);
            while (ShowCursor(true) < 0);
        }

        activeWindow = (HWND)lParam;
        return 0;
    }
    case WM_SIZE:
    {
        if (activeWindow == hChild)
        {
            nb::Core::EngineSettings::setHeight(HIWORD(lParam));
            nb::Core::EngineSettings::setWidth(LOWORD(lParam));
        }
        break;
    }
    case WM_CREATE:
        // SetWindowText(hwnd, L"MDI Child");
        return 0;

    case WM_DESTROY:
        return 0;

    case WM_HSCROLL:
    {

        if ((HWND)lParam == slider->GetHandle())
        {
            int value = slider->GetValue();
            nb::Core::EngineSettings::setFov(value);
        }
    }

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
