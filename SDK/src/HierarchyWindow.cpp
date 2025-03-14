#include "HierarchyWindow.hpp"

#include "PropertiesWindow.hpp"

#include <string>
#include <CommCtrl.h>

namespace Editor
{
    HierarchyWindow::HierarchyWindow(const HWND &parentHwnd, std::shared_ptr<nb::Core::Engine> engine)
        : engine(engine)
    {
        sEngine = engine;
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
        mcs.szTitle = L"HIERARCHY";
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
        // LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
        // style &= ~WS_CAPTION;
        // style &= ~WS_BORDER;
        // SetWindowLongPtr(hwnd, GWL_STYLE, style);

        RECT rcClient; // dimensions of client area
        InitCommonControls();

        GetClientRect(hwnd, &rcClient);
        tv = new WinAPITreeView(hwnd, 50, 50, 300, 300);
    }

    void HierarchyWindow::resize(const int newWidth, const int newHeight) noexcept
    {
        int width = newWidth * WIDTH_PROPORTION;
        int height = newHeight * HEIGHT_PROPORTION;

        MoveWindow(hwnd, newWidth - width, 0, width, height, TRUE);
    }

    void HierarchyWindow::update() noexcept
    {
        tv->AddNodesRecursive(engine->getScene()->getScene());
    }

    LRESULT HierarchyWindow::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        tv->HandleMessage(hwnd, message, wParam, lParam);

        switch (message)
        {

        case WM_CREATE:
            SetTimer(hwnd, 1, 20, NULL);
            break;

        case WM_TIMER:
            InvalidateRect(hwnd, NULL, FALSE);
            break;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            //
            RECT rect1;
            rect1.left = 100;
            rect1.top = 0;
            rect1.right = 500;
            rect1.bottom = 30;

            RECT rect2;
            rect2.left = 100;
            rect2.top = 0;
            rect2.right = 500;
            rect2.bottom = 60;

            RECT rect3;
            rect3.left = 100;
            rect3.top = 0;
            rect3.right = 500;
            rect3.bottom = 90;
            // GetClientRect(hwnd, &rect);
            if (sEngine != nullptr)
                pos = sEngine->getCameraPos();

            std::wstring camPos = std::to_wstring(pos.x) + L" " + std::to_wstring(pos.y) + L" " + std::to_wstring(pos.z);

            nb::Math::Vector3<float> camDir;
            if (sEngine != nullptr)
                camDir = sEngine->getCameraDirection();


            std::wstring camDirStr = std::to_wstring(camDir.x) + L" " + std::to_wstring(camDir.y) + L" " + std::to_wstring(camDir.z);
            std::wstring countOfDraws = L"Count of draws: " + std::to_wstring(nb::OpenGl::OpenGLRender::countOfDraws);


            DrawText(hdc, camPos.c_str(), -1, &rect1,
                     DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);

            DrawText(hdc, camDirStr.c_str(), -1, &rect2,
                     DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);            
                     
            DrawText(hdc, countOfDraws.c_str(), -1, &rect3,
                DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_DESTROY:
            return 0;
        case WM_NOTIFY:
        {
            LPNMHDR nmhdr = (LPNMHDR)lParam;

            if (nmhdr->hwndFrom == GetDlgItem(hwnd, 1) && nmhdr->code == TVN_SELCHANGED)
            {
                NMTREEVIEW *nmTreeView = (NMTREEVIEW *)lParam;
                HTREEITEM hSelectedItem = nmTreeView->itemNew.hItem;

                // Get the item text
                wchar_t buffer[256];
                TVITEM tvi = {0};
                tvi.mask = TVIF_TEXT | TVIF_HANDLE;
                tvi.hItem = hSelectedItem;
                tvi.pszText = buffer;
                tvi.cchTextMax = sizeof(buffer);
                if(TreeView_GetItem(nmhdr->hwndFrom, &tvi))
                {
                    std::wstring wStr(buffer);
                    selectedObjectName = std::string(wStr.begin(), wStr.end());
                    PropertiesWindow::setCurrentObject(sEngine->getScene()->find(selectedObjectName));
                }
                
            }
            return 0;
        }

        default:
            return DefMDIChildProc(hwnd, message, wParam, lParam);
        }
    }

    nb::Math::Vector3<float> HierarchyWindow::pos = {0.0f, 0.0f, 0.0f};
    std::shared_ptr<nb::Core::Engine> HierarchyWindow::sEngine = nullptr;
    std::string HierarchyWindow::selectedObjectName = "";
    WinAPITreeView *HierarchyWindow::tv = nullptr;
};
