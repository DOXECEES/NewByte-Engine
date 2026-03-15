#include "AssetManger.hpp"
#include <memory>

AssetManager::AssetManager()
{
    window = std::make_shared<Win32Window::ChildWindow>(nullptr);
    window->addCaption();

    window->getLayoutRoot()->addChild(buildUI());
    window->show();
}