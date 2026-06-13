#ifndef SDK_SCENE_SCENEWINDOW_HPP
#define SDK_SCENE_SCENEWINDOW_HPP

#include <Localization/Translation.hpp>
#include <Win32Window/Win32ChildWindow.hpp>
#include <memory>
#include <string>

namespace nbui
{
    class LayoutBuilder;
};

namespace sdk
{
    class SceneController;

    namespace UIConstants
    {
        constexpr int TOOLBAR_HEIGHT = 35;
    }

    class SceneWindow
    {
    public:
        SceneWindow(WindowInterface::IWindow* parent) noexcept;
        ~SceneWindow() noexcept = default;

        void initialize() noexcept;
        void handleResize(const NbRect<int>& rect) noexcept;

        void attachController(std::shared_ptr<SceneController> controller) noexcept;

        nbui::LayoutBuilder buildToolbar() noexcept;

        [[nodiscard]] std::shared_ptr<Win32Window::ChildWindow> getTabWindow() const noexcept
        {
            return sceneTabWindow;
        }
        [[nodiscard]] std::shared_ptr<Win32Window::ChildWindow> getToolbarWindow() const noexcept
        {
            return sceneToolbar;
        }
        [[nodiscard]] std::shared_ptr<Win32Window::ChildWindow> getViewportWindow() const noexcept
        {
            return sceneViewportWindow;
        }

    private:
        WindowInterface::IWindow*                 parentWindow = nullptr;
        std::shared_ptr<Win32Window::ChildWindow> sceneTabWindow;
        std::shared_ptr<Win32Window::ChildWindow> sceneToolbar;
        std::shared_ptr<Win32Window::ChildWindow> sceneViewportWindow;

        std::shared_ptr<SceneController>          sceneController;
    };
} // namespace sdk
#endif // SCENE_WINDOW_HPP