#include "SceneWindow.hpp"
#include <Common/StringUtils.hpp>

#include <Error/ErrorManager.hpp>

namespace sdk
{
    SceneWindow::SceneWindow(WindowInterface::IWindow* parent) noexcept : parentWindow(parent)
    {
    }

    void SceneWindow::initialize() noexcept
    {
        if (!parentWindow)
        {
            nb::Error::ErrorManager::instance().report(
                nb::Error::Type::FATAL, "Parent window is null during SceneWindow initialization"
            );
            return;
        }

        sceneTabWindow = std::make_shared<Win32Window::ChildWindow>(parentWindow, true);
        sceneTabWindow->setTitle(
            Utils::toWstring(Localization::Translation::fromKey("Ui.Editor.SceneTab.Title"))
        );

        sceneToolbar = std::make_shared<Win32Window::ChildWindow>(sceneTabWindow.get());
        sceneToolbar->setTitle(
            Utils::toWstring(Localization::Translation::fromKey("Ui.Editor.SceneToolbar.Title"))
        );

        sceneViewportWindow =
            std::make_shared<Win32Window::ChildWindow>(sceneTabWindow.get(), true);
        sceneViewportWindow->setTitle(
            Utils::toWstring(Localization::Translation::fromKey("Ui.Editor.Scene.Title"))
        );

        nb::Error::ErrorManager::instance().report(
            nb::Error::Type::INFO, "SceneWindow components created successfully"
        );
    }

    void SceneWindow::handleResize(const NbRect<int>& rect) noexcept
    {
        if (!sceneTabWindow || !sceneToolbar || !sceneViewportWindow)
        {
            return;
        }

        const NbPoint<int>& scenePos  = sceneTabWindow->getPosition();
        const NbSize<int>&  sceneSize = sceneTabWindow->getSize();

        sceneToolbar->setPosition({scenePos.x, scenePos.y});
        sceneToolbar->setSize({sceneSize.width, UIConstants::TOOLBAR_HEIGHT});

        sceneViewportWindow->setPosition({scenePos.x, scenePos.y + UIConstants::TOOLBAR_HEIGHT});
        sceneViewportWindow->setSize(
            {sceneSize.width, sceneSize.height - UIConstants::TOOLBAR_HEIGHT}
        );

        HWND viewportHandle = sceneViewportWindow->getHandle().as<HWND>();
        if (viewportHandle)
        {
            LONG_PTR viewportStyle = GetWindowLongPtr(viewportHandle, GWL_STYLE);
            SetWindowLongPtr(
                viewportHandle, GWL_STYLE, viewportStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN
            );
        }
    }
} // namespace sdk