#ifndef SRC_HIERARCHYWINDOW_HPP
#define SRC_HIERARCHYWINDOW_HPP

#include <Windows.h>

#include <../../Engine/src/Core/Engine.hpp>

namespace Editor
{
    class HierarchyWindow
    {
        static constexpr auto CLASS_NAME = L"MDI_HIERARCHY_CLASS";
        static constexpr auto WIDTH_PROPORTION = 0.3f;
        static constexpr auto HEIGHT_PROPORTION = 0.5f;

        public:
            HierarchyWindow(const HWND &parentHwnd, std::shared_ptr<nb::Core::Engine> engine);
            ~HierarchyWindow() = default;

            void resize(const int newWidth, const int newHeight) noexcept;

            inline const HWND &getHandle() const noexcept { return hwnd; };

            static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
            //void setPos(const nb::Math::Vector3<float> &p) { pos = p; };

        private:
            HWND hwnd;
            static nb::Math::Vector3<float> pos;

            std::shared_ptr<nb::Core::Engine> engine;
            static std::shared_ptr<nb::Core::Engine> sEngine;

    };
};

#endif

