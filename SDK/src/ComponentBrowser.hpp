#pragma once

#include <algorithm>
#include <d2d1.h>
#include <dwrite.h>
#include <functional>
#include <string>
#include <vector>
#include <windows.h>
#include <windowsx.h>
#include <wrl/client.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

#include "Renderer/FactorySingleton.hpp"

namespace nbui
{
    struct BrowserItem
    {
        std::wstring          fullPath;    // nb::Physics::Rigidbody
        std::wstring          displayName; // Rigidbody
        std::wstring          category;    // Physics
        std::function<void()> callback;

        BrowserItem(
            const std::wstring&   path,
            std::function<void()> cb
        )
            : fullPath(path)
            , callback(cb)
        {
            size_t lastColons = path.find_last_of(L":");
            if (lastColons != std::wstring::npos && lastColons > 0)
            {
                displayName = path.substr(lastColons + 1);

                size_t prevColons = path.find_last_of(L":", lastColons - 2);
                if (prevColons != std::wstring::npos)
                {
                    category = path.substr(prevColons + 1, lastColons - prevColons - 2);
                }
                else
                {
                    category = path.substr(0, lastColons - 1);
                }
            }
            else
            {
                displayName = path;
                category    = L"Common";
            }
        }
    };

    class ComponentBrowser
    {
    public:
        ComponentBrowser() : factory(Renderer::FactorySingleton::getFactory().Get())
        {
        }

        ~ComponentBrowser()
        {
            if (renderTarget)
            {
                renderTarget->Release();
            }
            if (hwnd)
            {
                DestroyWindow(hwnd);
            }
        }

        void init(HWND parent)
        {
            this->parentHwnd = parent;

            WNDCLASS wc      = {0};
            wc.lpfnWndProc   = WndProc;
            wc.hInstance     = GetModuleHandle(nullptr);
            wc.lpszClassName = L"NBUI_COMPONENT_BROWSER";
            wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

            RegisterClass(&wc);

            hwnd = CreateWindowEx(
                WS_EX_TOPMOST | WS_EX_TOOLWINDOW, wc.lpszClassName, L"", WS_POPUP, 0, 0, 0, 0,
                parentHwnd, nullptr, wc.hInstance, this
            );
        }

        void addItem(
            const std::wstring&   path,
            std::function<void()> cb
        )
        {
            allItems.emplace_back(path, cb);
        }

        void clear()
        {
            allItems.clear();
        }

        static ComponentBrowser& get()
        {
            static ComponentBrowser instance;
            return instance;
        }

        void show(
            HWND parent,
            int  x,
            int  y
        )
        {
            if (!hwnd)
            {
                this->init(parent);
            }

            searchString.clear();
            hoveredIndex = -1;
            updateFilter();
            updateWindowSize(x, y);

            ShowWindow(hwnd, SW_SHOW);
            SetFocus(hwnd);
            render();
        }


        void hide()
        {
            ShowWindow(hwnd, SW_HIDE);
        }

    private:
        HWND hwnd       = nullptr;
        HWND parentHwnd = nullptr;

        ID2D1Factory*                  factory      = nullptr;
        mutable ID2D1HwndRenderTarget* renderTarget = nullptr;

        mutable Microsoft::WRL::ComPtr<IDWriteTextFormat> textFormat;
        mutable Microsoft::WRL::ComPtr<IDWriteTextFormat> categoryFormat;

        mutable Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> textBrush;
        mutable Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> categoryBrush;
        mutable Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> hoverBrush;
        mutable Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> accentBrush;

        std::vector<BrowserItem>    allItems;
        mutable std::vector<size_t> filteredIndices;
        mutable std::wstring        searchString;

        const int   itemHeight   = 26;
        const int   searchHeight = 36;
        const float width        = 280.0f;
        mutable int hoveredIndex = -1;

        void updateFilter() const
        {
            filteredIndices.clear();
            std::wstring searchLower = searchString;
            std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::towlower);

            for (size_t i = 0; i < allItems.size(); ++i)
            {
                if (searchString.empty())
                {
                    filteredIndices.push_back(i);
                    continue;
                }

                std::wstring nameLower = allItems[i].fullPath;
                std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::towlower);

                if (nameLower.find(searchLower) != std::wstring::npos)
                {
                    filteredIndices.push_back(i);
                }
            }
        }

        void updateWindowSize(
            int x = -1,
            int y = -1
        ) const
        {
            int totalHeight = searchHeight + (int(filteredIndices.size()) * itemHeight) + 10;
            if (filteredIndices.empty())
            {
                totalHeight = searchHeight + 40;
            }

            if (x == -1)
            {
                RECT rc;
                GetWindowRect(hwnd, &rc);
                x = rc.left;
                y = rc.top;
            }

            SetWindowPos(hwnd, HWND_TOPMOST, x, y, (int)width, totalHeight, SWP_NOACTIVATE);
            if (renderTarget)
            {
                renderTarget->Resize(D2D1_SIZE_U((UINT32)width, (UINT32)totalHeight));
            }
        }

        static LRESULT CALLBACK WndProc(
            HWND   hwnd,
            UINT   msg,
            WPARAM wParam,
            LPARAM lParam
        )
        {
            ComponentBrowser* self = (ComponentBrowser*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

            switch (msg)
            {
            case WM_CREATE:
                SetWindowLongPtr(
                    hwnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT*)lParam)->lpCreateParams
                );
                return 0;

            case WM_CHAR:
                if (wParam == VK_BACK)
                {
                    if (!self->searchString.empty())
                    {
                        self->searchString.pop_back();
                    }
                }
                else if (wParam == VK_ESCAPE)
                {
                    self->hide();
                }
                else if (wParam >= 32)
                {
                    self->searchString += (wchar_t)wParam;
                }
                self->updateFilter();
                self->updateWindowSize();
                InvalidateRect(hwnd, nullptr, FALSE);
                return 0;

            case WM_MOUSEMOVE:
            {
                int mouseY   = GET_Y_LPARAM(lParam) - self->searchHeight;
                int newIndex = (mouseY < 0) ? -1 : (mouseY / self->itemHeight);
                if (newIndex >= (int)self->filteredIndices.size())
                {
                    newIndex = -1;
                }

                if (newIndex != self->hoveredIndex)
                {
                    self->hoveredIndex = newIndex;
                    InvalidateRect(hwnd, nullptr, FALSE);
                }
                return 0;
            }

            case WM_LBUTTONDOWN:
                if (self->hoveredIndex != -1)
                {
                    self->allItems[self->filteredIndices[self->hoveredIndex]].callback();
                    self->hide();
                }
                return 0;

            case WM_KILLFOCUS:
                self->hide();
                return 0;

            case WM_PAINT:
                self->render();
                ValidateRect(hwnd, nullptr);
                return 0;
            }
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }

        void ensureResources() const
        {
            if (renderTarget)
            {
                return;
            }

            RECT rc;
            GetClientRect(hwnd, &rc);
            D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
            factory->CreateHwndRenderTarget(
                D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hwnd, size),
                &renderTarget
            );

            renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.9f, 0.9f, 0.9f), &textBrush);
            renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.5f, 0.5f, 0.5f), &categoryBrush);
            renderTarget->CreateSolidColorBrush(
                D2D1::ColorF(1.0f, 0.6f, 0.0f), &accentBrush
            ); 
            renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.25f, 0.25f, 0.25f), &hoverBrush);

            auto writeFactory = Renderer::FactorySingleton::getDirectWriteFactory();
            writeFactory->CreateTextFormat(
                L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL, 14.0f, L"ru-RU", &textFormat
            );
            writeFactory->CreateTextFormat(
                L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL, 11.0f, L"ru-RU", &categoryFormat
            );

            textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
            categoryFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        }

        void render() const
        {
            ensureResources();
            if (!renderTarget)
            {
                return;
            }

            renderTarget->BeginDraw();
            renderTarget->Clear(D2D1::ColorF(0.15f, 0.15f, 0.15f));

            D2D1_RECT_F  searchRect = D2D1::RectF(10, 0, width - 10, (float)searchHeight);
            std::wstring hint       = searchString.empty() ? L"Search component..." : searchString;
            renderTarget->DrawTextW(
                hint.c_str(), (UINT32)hint.size(), textFormat.Get(), searchRect,
                searchString.empty() ? categoryBrush.Get() : accentBrush.Get()
            );

            renderTarget->DrawLine(
                D2D1::Point2F(0, (float)searchHeight), D2D1::Point2F(width, (float)searchHeight),
                hoverBrush.Get(), 1.0f
            );

            if (filteredIndices.empty())
            {
                D2D1_RECT_F emptyRect =
                    D2D1::RectF(0, (float)searchHeight, width, (float)searchHeight + 40);
                renderTarget->DrawTextW(
                    L"No results", 10, textFormat.Get(), emptyRect, categoryBrush.Get()
                );
            }
            else
            {
                for (int i = 0; i < (int)filteredIndices.size(); ++i)
                {
                    const auto& item = allItems[filteredIndices[i]];
                    float       top  = (float)(searchHeight + i * itemHeight);
                    D2D1_RECT_F rect = D2D1::RectF(0, top, width, top + itemHeight);

                    if (i == hoveredIndex)
                    {
                        renderTarget->FillRectangle(rect, hoverBrush.Get());
                    }

                    D2D1_RECT_F nameRect = rect;
                    nameRect.left += 12;
                    renderTarget->DrawTextW(
                        item.displayName.c_str(), (UINT32)item.displayName.size(), textFormat.Get(),
                        nameRect, textBrush.Get()
                    );

                    D2D1_RECT_F catRect = rect;
                    catRect.right -= 12;
                    categoryFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
                    renderTarget->DrawTextW(
                        item.category.c_str(), (UINT32)item.category.size(), categoryFormat.Get(),
                        catRect, categoryBrush.Get()
                    );
                }
            }

            renderTarget->DrawRectangle(
                D2D1::RectF(
                    0.5f, 0.5f, width - 0.5f,
                    (float)(searchHeight + filteredIndices.size() * itemHeight + 10)
                ),
                hoverBrush.Get(), 1.0f
            );

            renderTarget->EndDraw();
        }
    };
} // namespace nbui