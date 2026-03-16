#include "AssetManger.hpp"
#include <memory>

AssetManager::AssetManager(nbstl::NonOwningPtr<nb::Core::Engine> engine) 
    : engine(engine)
{
    importAsset("Assets/res/brick.png");
    window = std::make_shared<Win32Window::ChildWindow>(nullptr);
    window->addCaption();

    window->getLayoutRoot()->addChild(buildUI());
    window->show();
}