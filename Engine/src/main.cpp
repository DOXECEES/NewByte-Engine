//
// #include <Windows.h>
//
// #include <direct.h>
//
// int WINAPI WinMain(
//    HINSTANCE hInstance,
//    HINSTANCE hPrevInstance,
//    LPSTR lpCmdLine,
//    int nCmdShow
//)
//{
//    HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
//
//    if (SUCCEEDED(CoInitialize(nullptr)))
//    {
//        // init subsystems
//        Subsystems.init()
//
//
//
//        // render window
//
//
//
//
//
//
//
//    }
//
//    return 0;
//}

#include <windows.h>
#include <d2d1.h>
#include <sstream>

#include "Subsystems.hpp"

#include "Window.hpp"
#include "UI/Button.hpp"

#include "Debug.hpp"
#include "Math/Vector2.hpp"

#include "Loaders/BitmapLoader.hpp"
#include "Loaders/PngLoader.hpp"

#include "Templates/BinaryTree.hpp"

#pragma comment(lib, "d2d1.lib")


Button *b = new Button(nb::Math::Vector2{100, 100}, 200, 200, "hello");
Button *b1 = new Button(nb::Math::Vector2{0, 0}, 50, 50, "hello");

LRESULT _stdcall WndProc(HWND hWnd, UINT message,
                         WPARAM wParam, LPARAM lParam);

Renderer2D *renderer2D = nullptr;

class Example
{
public:
    Example() = default;

#ifdef _DEBUG
    friend std::ostream &operator<<(std::ostream &outs, const Example &p);
#endif // DEBUG

private:
    int a = 1;
    int b = 2;
};

#ifdef _DEBUG
std::ostream &operator<<(std::ostream &outs, const Example &p)
{
    return outs << "(" << p.a << "," << p.b << ")";
}
#endif // DEBUG

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

    AllocConsole();

    auto systems = new Subsystems();
    systems->Init();

    renderer2D = systems->GetRenderer2D();

    Window *w = new Window(hInstance, WndProc);

#ifdef _DEBUG

    std::deque<Example> a = {Example(), Example()};

    // nb::Loaders::BitmapLoader("C:\\Install Programms\\sample1.bmp");
    //nb::Loaders::PngLoader p("C:\\Install Programms\\a.png");
   // Debug::debug(p.good());

    nb::Templates::BinaryTree<int> tree;
    tree.insert(1);
    tree.insert(2);
    tree.insert(3);
    tree.insert(4);
    tree.insert(5);
    tree.insert(6);
    tree.insert(7);

    tree.preOrder([](int x)
                 { Debug::debug(x); });

    Debug::debug(static_cast<uint16_t>(tree.depth()));
    // Debug::debug(a);
    // Debug::debug(a);

    // nb::Math::Vector2 vec1(21.1f, 0.2f);
    // auto vec2 = nb::Math::Vector2(21.0f, 5.333f);
    // vec2 = vec2 + vec1;
    // auto scal = vec2.dot(vec1);

    // Debug::debug(scal);

#endif // _DEBUG

    while (w->render())
    {
    }

    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg,
                         WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_PAINT:
        renderer2D->beginPaint(hWnd);
        renderer2D->drawUi(b1);
        renderer2D->drawUi(b);
        renderer2D->endPaint();
        return 0;
    case WM_SIZE:
        renderer2D->resize(LOWORD(lParam), HIWORD(lParam));
        InvalidateRect(hWnd, NULL, FALSE);
    case WM_LBUTTONDOWN:
    {
        auto x = LOWORD(lParam);
        auto y = HIWORD(lParam);
        POINT pos = {x, y};

        if (b->onClick(nb::Math::Vector2{static_cast<float>(pos.x), static_cast<float>(pos.y)}))
        {
            renderer2D->changeBrush();
        }

        InvalidateRect(hWnd, NULL, FALSE);

        return 0;
    }
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
}

//
// #include <windows.h>
//
// LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//
// int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
//    auto CLASS_NAME = L"Sample Window Class";
//
//    WNDCLASS wc = { };
//
//    wc.lpfnWndProc = WindowProc;
//    wc.hInstance = hInstance;
//    wc.lpszClassName = CLASS_NAME;
//    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
//
//    RegisterClass(&wc);
//
//    HWND hwnd = CreateWindowEx(
//        0,
//        CLASS_NAME,
//        L"Dark Themed Button",
//        WS_OVERLAPPEDWINDOW,
//        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
//        NULL,
//        NULL,
//        hInstance,
//        NULL
//    );
//
//    if (hwnd == NULL) {
//        return 0;
//    }
//
//    ShowWindow(hwnd, nCmdShow);
//
//    MSG msg = { };
//    while (GetMessage(&msg, NULL, 0, 0)) {
//        TranslateMessage(&msg);
//        DispatchMessage(&msg);
//    }
//
//    return 0;
//}
//
// void DrawDarkButton(LPDRAWITEMSTRUCT lpDrawItemStruct) {
//    //HDC hdc = lpDrawItemStruct->hDC;
//    //RECT rc = lpDrawItemStruct->rcItem;
//    //UINT state = lpDrawItemStruct->itemState;
//
//    //// Draw the button background
//    //SetBkColor(hdc, RGB(45, 45, 48));
//    //HBRUSH hBrush = CreateSolidBrush(RGB(45, 45, 48));
//    //FillRect(hdc, &rc, hBrush);
//    //DeleteObject(hBrush);
//
//    //// Draw the button text
//
//    ///*HFONT font = CreateFont(0, 0, GM_ADVANCED, 0, FW_DONTCARE,
//    //    FALSE, FALSE, FALSE, ANSI_CHARSET,
//    //    OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DRAFT_QUALITY, VARIABLE_PITCH,
//    //    TEXT("Tekton Pro"));
//    //SelectObject(hdc, font);*/
//    //SetTextColor(hdc, RGB(255, 255, 255));
//    //SetBkMode(hdc, TRANSPARENT);
//    //
//    //DrawText(hdc, L"Click Me", -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
//    //
//    //HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0)); // Red border
//    //HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
//    //HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
//
//    //Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
//
//    //SelectObject(hdc, hOldPen);
//    //SelectObject(hdc, hOldBrush);
//    //DeleteObject(hPen);
//
//    //// Optionally draw a sunken border if the button is pressed
//    //if (lpDrawItemStruct->itemState & ODS_SELECTED) {
//    //    DrawEdge(hdc, &rc, EDGE_SUNKEN, BF_RECT);
//    //}
//    //else {
//    //    DrawEdge(hdc, &rc, EDGE_RAISED, BF_RECT);
//    //}
//
//    HDC hdc = lpDrawItemStruct->hDC;
//    RECT rc = lpDrawItemStruct->rcItem;
//
//    // Draw the button background
//    HBRUSH hBrush = CreateSolidBrush(RGB(80, 79, 79)); // Cornflower Blue
//    FillRect(hdc, &rc, hBrush);
//    DeleteObject(hBrush);
//
//    // Draw the button text
//    SetTextColor(hdc, RGB(255, 255, 255)); // White text
//    SetBkMode(hdc, TRANSPARENT);
//    DrawText(hdc, L"Custom Button", -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
//
//    // Draw the button border with a custom color
//    HPEN hPen = CreatePen(PS_SOLID, 4, RGB(0, 0, 0)); // Red border
//    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
//    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
//
//    // Adjust rectangle size to avoid clipping with the border
//    Rectangle(hdc, rc.left-1, rc.top-1, rc.right-1, rc.bottom-1);
//
//    SelectObject(hdc, hOldPen);
//    SelectObject(hdc, hOldBrush);
//    DeleteObject(hPen);
//
//
//    if (lpDrawItemStruct->itemState & ODS_SELECTED) {
//        DrawEdge(hdc, &rc, EDGE_SUNKEN, BF_RECT);
//    }
//    else {
//        DrawEdge(hdc, &rc, EDGE_SUNKEN, BF_RECT);
//    }
//}
//
// LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
//    static HWND hwndButton;
//    switch (uMsg) {
//    case WM_CREATE:
//        hwndButton = CreateWindow(
//            L"BUTTON",
//            L"Click Me",
//            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
//            10,
//            10,
//            100,
//            30,
//            hwnd,
//            (HMENU)1,
//            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
//            NULL
//        );
//        break;
//    case WM_DRAWITEM:
//        if (wParam == 1) {
//            LPDRAWITEMSTRUCT lpDrawItemStruct = (LPDRAWITEMSTRUCT)lParam;
//            DrawDarkButton(lpDrawItemStruct);
//        }
//        break;
//    case WM_DESTROY:
//        PostQuitMessage(0);
//        return 0;
//    }
//    return DefWindowProc(hwnd, uMsg, wParam, lParam);
//}
