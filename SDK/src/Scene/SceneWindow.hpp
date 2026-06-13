#ifndef SDK_SCENE_SCENEWINDOW_HPP
#define SDK_SCENE_SCENEWINDOW_HPP

#include <Localization/Translation.hpp>
#include <Win32Window/Win32ChildWindow.hpp>
#include <memory>
#include <string>

namespace sdk
{

    namespace UIConstants
    {
        constexpr int TOOLBAR_HEIGHT = 35;
    }

    class SceneWindow
    {
    public:
        explicit SceneWindow(WindowInterface::IWindow* parent) noexcept;
        ~SceneWindow() noexcept = default;

        void initialize() noexcept;
        void handleResize(const NbRect<int>& rect) noexcept;

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
    };
} // namespace sdk
#endif // SCENE_WINDOW_HPP