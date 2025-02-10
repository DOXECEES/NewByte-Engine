#ifndef SRC_HIERARCHYWINDOW_HPP
#define SRC_HIERARCHYWINDOW_HPP

#include <Windows.h>
#include <commctrl.h>

#include <../../Engine/src/Core/Engine.hpp>

namespace Editor
{
    class WinAPITreeView
    { 
    public:
        WinAPITreeView(HWND parent, int x, int y, int width, int height)
            : parentWindow(parent), treeViewHandle(nullptr), draggingItem(nullptr)
        {
            INITCOMMONCONTROLSEX icex;
            icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
            icex.dwICC = ICC_TREEVIEW_CLASSES;
            InitCommonControlsEx(&icex);

            treeViewHandle = CreateWindowEx(
                0, WC_TREEVIEW, TEXT(""),
                WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS,
                x, y, width, height, parentWindow, (HMENU)1, GetModuleHandle(nullptr), nullptr);
        }

        HWND GetHandle() const
        {
            return treeViewHandle;
        }

        HTREEITEM AddItem(const std::wstring &text, HTREEITEM parent = nullptr)
        {
            TVINSERTSTRUCT tvis = {};
            tvis.hParent = parent;
            tvis.hInsertAfter = TVI_LAST;
            tvis.item.mask = TVIF_TEXT;
            tvis.item.pszText = const_cast<LPWSTR>(text.c_str());

            return TreeView_InsertItem(treeViewHandle, &tvis);
        }

        void AddNodesRecursive(std::shared_ptr<nb::Renderer::BaseNode> node, HTREEITEM parent = nullptr)
        {
            if (!node)
                return;

            std::wostringstream conv;
            conv << node->getName().c_str();

            HTREEITEM currentItem = AddItem(conv.str(), parent);

            for (const auto &child : node->getChildrens())
            {
                AddNodesRecursive(child, currentItem);
            }
        }

        void HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
        {
            switch (msg)
            {
            case WM_LBUTTONDOWN:
                OnLButtonDown(wParam, lParam);
                break;

            case WM_MOUSEMOVE:
                OnMouseMove(wParam, lParam);
                break;

            case WM_LBUTTONUP:
                OnLButtonUp(wParam, lParam);
                break;

            default:
                break;
            }
        }

    private:
        HWND parentWindow;
        HWND treeViewHandle;
        HTREEITEM draggingItem;

        void OnLButtonDown(WPARAM wParam, LPARAM lParam)
        {
            if (wParam & MK_LBUTTON)
            {
                TVHITTESTINFO hitTest = {};
                hitTest.pt.x = LOWORD(lParam);
                hitTest.pt.y = HIWORD(lParam);

                if (TreeView_HitTest(treeViewHandle, &hitTest) && hitTest.hItem)
                {
                    draggingItem = hitTest.hItem;
                }
            }
        }

        void OnMouseMove(WPARAM wParam, LPARAM lParam)
        {
            if (draggingItem && (wParam & MK_LBUTTON))
            {
                // Можно визуализировать индикатор перемещения здесь или показать некоторые дополнительные элементы
                // Обновляем состояние мыши при перемещении
            }
        }

        void OnLButtonUp(WPARAM wParam, LPARAM lParam)
        {
            if (draggingItem)
            {
                TVHITTESTINFO hitTest = {};
                hitTest.pt.x = LOWORD(lParam);
                hitTest.pt.y = HIWORD(lParam);

                HTREEITEM targetItem = TreeView_HitTest(treeViewHandle, &hitTest) ? hitTest.hItem : nullptr;

                if (targetItem && targetItem != draggingItem)
                {
                    // Перемещение узла
                    MoveItem(draggingItem, targetItem);
                }

                draggingItem = nullptr;
            }
        }

        void MoveItem(HTREEITEM sourceItem, HTREEITEM targetItem)
        {
            if (!sourceItem || !targetItem)
                return;

            // Получаем текст узла-источника
            WCHAR buffer[256] = {};
            TVITEM tvi = {};
            tvi.hItem = sourceItem;
            tvi.mask = TVIF_TEXT;
            tvi.pszText = buffer;
            tvi.cchTextMax = sizeof(buffer) / sizeof(WCHAR);

            // Попытка извлечь текст
            if (!TreeView_GetItem(treeViewHandle, &tvi))
            {
                MessageBox(nullptr, L"Не удалось получить текст узла", L"Ошибка", MB_OK);
                return;
            }

            // Вставляем новый узел в целевое место
            TVINSERTSTRUCT tvis = {};
            tvis.hParent = targetItem;
            tvis.hInsertAfter = TVI_LAST;
            tvis.item.mask = TVIF_TEXT;
            tvis.item.pszText = buffer;

            HTREEITEM newItem = TreeView_InsertItem(treeViewHandle, &tvis);
            if (!newItem)
            {
                MessageBox(nullptr, L"Не удалось переместить узел", L"Ошибка", MB_OK);
                return;
            }

            // Удаляем старый узел после успешного перемещения
            TreeView_DeleteItem(treeViewHandle, sourceItem);
        }
    };

    class HierarchyWindow
    {
        static constexpr auto CLASS_NAME = L"MDI_HIERARCHY_CLASS";
        static constexpr auto WIDTH_PROPORTION = 0.3f;
        static constexpr auto HEIGHT_PROPORTION = 0.5f;

    public:
        HierarchyWindow(const HWND &parentHwnd, std::shared_ptr<nb::Core::Engine> engine);
        ~HierarchyWindow() = default;

        void resize(const int newWidth, const int newHeight) noexcept;
        void update() noexcept;

        inline const HWND &getHandle() const noexcept { return hwnd; };

        inline static const std::string &getSelectedObjectName() noexcept { return selectedObjectName; };
        static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
        // void setPos(const nb::Math::Vector3<float> &p) { pos = p; };

    private:
        HWND hwnd;
        static WinAPITreeView *tv;
        static nb::Math::Vector3<float> pos;

        std::shared_ptr<nb::Core::Engine> engine;
        static std::shared_ptr<nb::Core::Engine> sEngine;

        static std::string selectedObjectName;
    };
};

#endif
