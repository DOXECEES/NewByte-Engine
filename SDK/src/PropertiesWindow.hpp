#ifndef SRC_PROPERTIESWINDOW_HPP
#define SRC_PROPERTIESWINDOW_HPP

#include <Windows.h>

#include <../../Engine/src/Core/Engine.hpp>

#include "Ui/Slider.hpp"


namespace Editor
{
    class PropertiesWindow
    {
        static constexpr auto CLASS_NAME = L"MDI_PROPERTIES_CLASS";
        static constexpr auto WIDTH_PROPORTION = 0.3f;
        static constexpr auto HEIGHT_PROPORTION = 0.3f;

        public:
            PropertiesWindow(const HWND& parentHwnd);
            ~PropertiesWindow();

            void resize(const int newWidth, const int newHeigth) noexcept;

            inline const HWND &getHandle() const noexcept { return hwnd; };

            static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

        private:
            
            HWND hwnd;
            static nb::Math::Vector3<float> pos;

            static Ui::Slider *slider;

            static Ui::Slider *rAmbient;
            static Ui::Slider *gAmbient;
            static Ui::Slider *bAmbient;

            static Ui::Slider *xPos;
            static Ui::Slider *yPos;
            static Ui::Slider *zPos;


    };
};

#endif
