#ifndef SRC_PROPERTIESWINDOW_HPP
#define SRC_PROPERTIESWINDOW_HPP

#include <Windows.h>

#include <../../Engine/src/Core/Engine.hpp>

#include "Ui/Slider.hpp"
#include "Ui/ComboBox.hpp"

#include "Ui/RadioGroup.hpp"

namespace Editor
{
    class CoordinateEditor
    {
    public:
        CoordinateEditor(HWND parent, float x, float y, float z, int blockOffset = 0)
        {
            hEditX = CreateWindowEx(
                0, L"EDIT", std::to_wstring(x).c_str(),
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
                10 + blockOffset, 50, 100, 25,
                parent, NULL, NULL, NULL);

            hEditY = CreateWindowEx(
                0, L"EDIT", std::to_wstring(y).c_str(),
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
                10 + blockOffset, 90, 100, 25,
                parent, NULL, NULL, NULL);

            hEditZ = CreateWindowEx(
                0, L"EDIT", std::to_wstring(z).c_str(),
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
                10 + blockOffset, 130, 100, 25,
                parent, NULL, NULL, NULL);

            // Создание SpinBox контролов для X, Y и Z
            hSpinX = CreateWindowEx(
                0, UPDOWN_CLASS, NULL,
                WS_CHILD | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ARROWKEYS,
                120 + blockOffset, 50, 30, 25,
                parent, NULL, NULL, NULL);
            SendMessage(hSpinX, UDM_SETBUDDY, (WPARAM)hEditX, 0);
            SendMessage(hSpinX, UDM_SETRANGE, 0, MAKELPARAM(1000, -1000)); // Установка диапазона

            hSpinY = CreateWindowEx(
                0, UPDOWN_CLASS, NULL,
                WS_CHILD | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ARROWKEYS,
                120 + blockOffset, 90, 30, 25,
                parent, NULL, NULL, NULL);
            SendMessage(hSpinY, UDM_SETBUDDY, (WPARAM)hEditY, 0);
            SendMessage(hSpinY, UDM_SETRANGE, 0, MAKELPARAM(1000, -1000)); // Установка диапазона

            hSpinZ = CreateWindowEx(
                0, UPDOWN_CLASS, NULL,
                WS_CHILD | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ARROWKEYS,
                120 + blockOffset, 130, 30, 25,
                parent, NULL, NULL, NULL);
            SendMessage(hSpinZ, UDM_SETBUDDY, (WPARAM)hEditZ, 0);
            SendMessage(hSpinZ, UDM_SETRANGE, 0, MAKELPARAM(1000, -1000)); // Установка диапазона
        }

        void SetCoordinates(float x, float y, float z)
        {
            SetWindowText(hEditX, std::to_wstring(x).c_str());
            SetWindowText(hEditY, std::to_wstring(y).c_str());
            SetWindowText(hEditZ, std::to_wstring(z).c_str());
            SendMessage(hSpinX, UDM_SETPOS, 0, MAKELPARAM(static_cast<int>(x), 0));
            SendMessage(hSpinY, UDM_SETPOS, 0, MAKELPARAM(static_cast<int>(y), 0));
            SendMessage(hSpinZ, UDM_SETPOS, 0, MAKELPARAM(static_cast<int>(z), 0));
        }

        void GetCoordinates(float &x, float &y, float &z)
        {
            wchar_t buffer[16];
            GetWindowText(hEditX, buffer, 16);
            x = static_cast<float>(_wtof(buffer));
            GetWindowText(hEditY, buffer, 16);
            y = static_cast<float>(_wtof(buffer));
            GetWindowText(hEditZ, buffer, 16);
            z = static_cast<float>(_wtof(buffer));
        }

        float getX()
        {
            wchar_t buffer[16];
            GetWindowText(hEditX, buffer, 16);
            return static_cast<float>(_wtof(buffer));
        }

        float getY()
        {
            wchar_t buffer[16];
            GetWindowText(hEditY, buffer, 16);
            return static_cast<float>(_wtof(buffer));
        }

        float getZ()
        {
            wchar_t buffer[16];
            GetWindowText(hEditZ, buffer, 16);
            return static_cast<float>(_wtof(buffer));
        }



        HWND GetEditXHandle() const noexcept { return hEditX; }
        HWND GetEditYHandle() const noexcept { return hEditY; }
        HWND GetEditZHandle() const noexcept { return hEditZ; }
        HWND GetSpinXHandle() const noexcept { return hSpinX; }
        HWND GetSpinYHandle() const noexcept { return hSpinY; }
        HWND GetSpinZHandle() const noexcept { return hSpinZ; }

        void setCoordinates(const nb::Math::Vector3<float> &coords) noexcept
        {
            return SetCoordinates(coords.x, coords.y, coords.z);
        }

    private:
        HWND hEditX;
        HWND hEditY;
        HWND hEditZ;
        HWND hSpinX;
        HWND hSpinY;
        HWND hSpinZ;
    };

    class PropertiesWindow
    {
        static constexpr auto CLASS_NAME = L"MDI_PROPERTIES_CLASS";
        static constexpr auto WIDTH_PROPORTION = 0.3f;
        static constexpr auto HEIGHT_PROPORTION = 0.5f;

    public:
        PropertiesWindow(const HWND &parentHwnd, std::shared_ptr<nb::Core::Engine> engine);
        ~PropertiesWindow();
        

        void resize(const int newWidth, const int newHeigth) noexcept;

        inline const HWND &getHandle() const noexcept { return hwnd; };

        static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

        static void setCurrentObject(std::shared_ptr<nb::Renderer::BaseNode> obj) noexcept;

    private:
        HWND hwnd;

        static std::shared_ptr<nb::Renderer::BaseNode> currentObject;
        static Editor::CoordinateEditor *coordsEdit;
        static Editor::CoordinateEditor *rotateEdit;
        static Editor::CoordinateEditor *scaleEdit;

        static Ui::RadioGroup *radioShadingModel;

        static nb::Math::Vector3<float> pos;
        static std::shared_ptr<nb::Core::Engine> sEngine;

        static Ui::Slider *slider;

        static Ui::Slider *rAmbient;
        static Ui::Slider *gAmbient;
        static Ui::Slider *bAmbient;

        static Ui::Slider *xPos;
        static Ui::Slider *yPos;
        static Ui::Slider *zPos;

        static Ui::ComboBox *lightModel;
    };
};

#endif
