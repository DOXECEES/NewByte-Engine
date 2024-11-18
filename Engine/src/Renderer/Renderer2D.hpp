
// #pragma once

// #include <d2d1.h>

// #include "../UI/Button.hpp"

// #pragma comment(lib, "d2d1.lib")

// class Ui;

// class Renderer2D
// {

// public:
//     Renderer2D();
//     ~Renderer2D();

//     void createResources(HWND hwnd);
//     void resize(const UINT32 width, const UINT32 height) noexcept;
//     void changeBrush();

//     void drawQuad();
//     void drawUi(Ui *ui);

//     inline void beginPaint(HWND hwnd) noexcept
//     {
//         currentHwnd = hwnd;
//         BeginPaint(currentHwnd, &ps);
//         createResources(currentHwnd);
//         renderTarget->BeginDraw();
//     };

//     inline void endPaint() noexcept
//     {
//         HRESULT hr = renderTarget->EndDraw();
//         if (hr == D2DERR_RECREATE_TARGET)
//         {
//             if (rbrush)
//                 rbrush->Release();
//             if (renderTarget)
//                 renderTarget->Release();
//             rbrush = nullptr;
//             renderTarget = nullptr;
//         }

//         EndPaint(currentHwnd, &ps);
//         currentHwnd = {};
//     };

// private:
//     ID2D1Factory *factory = nullptr;
//     ID2D1HwndRenderTarget *renderTarget = nullptr;

//     ID2D1SolidColorBrush *rbrush = nullptr;
//     ID2D1SolidColorBrush *borderBrush = nullptr;

//     PAINTSTRUCT ps;

//     HWND currentHwnd = {};
// };
