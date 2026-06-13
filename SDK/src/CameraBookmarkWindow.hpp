#ifndef SDK_CAMERABOOKMARKWINDOW_HPP
#define SDK_CAMERABOOKMARKWINDOW_HPP

#include <NbCore.hpp>

#include <WindowInterface/IWindow.hpp>
#include <LayoutBuilder.hpp>

#include "CameraBookmark.hpp"
#include "Camera/CameraSettings.hpp"
#include "Camera/CameraSettingsController.hpp"

#include <string>

namespace sdk
{

    
    

    class CameraBookmarkWindow
    {
    public:
        CameraBookmarkWindow() noexcept = default;
        CameraBookmarkWindow(
            const std::shared_ptr<WindowInterface::IWindow>& parent,
            const CameraBookmarkManager& manager,
            const CameraSettingsController& controller
        ) noexcept;
        ~CameraBookmarkWindow() = default;
        NB_NON_COPYMOVABLE(CameraBookmarkWindow);

        std::shared_ptr<WindowInterface::IWindow> getWindow() const noexcept;
    
        void refreshUi() noexcept;

    private:
        nbui::LayoutBuilder buildUi() noexcept;

        struct CameraBookmarkModel
        {
            std::wstring name;
            float fov;
            float x, y, z;
            float yaw, pitch, roll;
            float transitionTime;
            bool isActive;
        };

        void selectBookmark(const CameraBookmarkModel& bookmark) noexcept;


    private:
        std::shared_ptr<WindowInterface::IWindow> window = nullptr;
        const CameraBookmarkManager& manager;
        const CameraSettingsController& controller;
        CameraBookmarkModel selectedItem;

        CameraSettings settings = {};

    };
    
};

#endif