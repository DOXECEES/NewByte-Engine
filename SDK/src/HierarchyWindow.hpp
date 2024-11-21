#ifndef SRC_HIERARCHYWINDOW_HPP
#define SRC_HIERARCHYWINDOW_HPP

#include <Windows.h>

namespace Editor
{
    class HierarchyWindow
    {
        static constexpr auto CLASS_NAME = L"MDI_HIERARCHY_CLASS";
        static constexpr auto WIDTH_PROPORTION = 0.3f;
        static constexpr auto HEIGHT_PROPORTION = 0.7f;

        public:
            HierarchyWindow(const HWND &parentHwnd);
            ~HierarchyWindow() = default;

            void resize(const int newWidth, const int newHeight) noexcept;

            inline const HWND &getHandle() const noexcept { return hwnd; };

            static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


        private:
            HWND hwnd;
    };
};

#endif

