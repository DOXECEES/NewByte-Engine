#ifndef SDK_FRAMEBUFFERVISUALIZATION_HPP
#define SDK_FRAMEBUFFERVISUALIZATION_HPP

#include <Core/Engine.hpp>
#include <NonOwningPtr.hpp>

#include <WindowInterface/IWindow.hpp>

#include <Vector.hpp>

#include <memory>

namespace NNsLayout
{
    class LayoutNode;
}

namespace Sdk
{
    class FramebufferVisualization
    {
    public:
        FramebufferVisualization(
            std::shared_ptr<WindowInterface::IWindow> mainWindow,
            nbstl::NonOwningPtr<nb::Core::Engine> engine
        ) noexcept;
    
        std::unique_ptr<NNsLayout::LayoutNode> buildUi() noexcept;
        void repositionGrid() noexcept;
        void buildWindows() noexcept;
        void addFramebuffer() noexcept;

    private:

        std::shared_ptr<WindowInterface::IWindow> mainWindow;
        nbstl::Vector<std::unique_ptr<WindowInterface::IWindow>> childrenWindows;
        nbstl::Vector<nb::Renderer::SharedWindowContext> sharedWindowContexts;        
        
        nbstl::NonOwningPtr<nb::Core::Engine> engine;

        std::unique_ptr<WindowInterface::IWindow> toolbarWindow;
        nbstl::Vector<std::unique_ptr<WindowInterface::IWindow>> headerWindows;
        nbstl::Vector<std::unique_ptr<WindowInterface::IWindow>> viewportWindows;

    };
}

#endif