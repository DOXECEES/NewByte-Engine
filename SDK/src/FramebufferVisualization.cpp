#include "FramebufferVisualization.hpp"

#include <Utils.hpp>

#include <LayoutBuilder.hpp>
#include <Widgets/Button.hpp>
#include <Widgets/ComboBox.hpp>
#include <Win32Window/Win32ChildWindow.hpp>

#include <Renderer/RendererTracker.hpp>
#include <string>

namespace Sdk
{
    static constexpr size_t DEFAULT_WINDOW_COUNT = 8;
    static constexpr int TOOLBAR_HEIGHT = 40;
    static constexpr int CELL_HEADER_HEIGHT = 25;

    FramebufferVisualization::FramebufferVisualization(
        std::shared_ptr<WindowInterface::IWindow> mainWindow,
        nbstl::NonOwningPtr<nb::Core::Engine>     engine
    ) noexcept
        : engine(engine)
    {
        this->mainWindow = std::make_shared<Win32Window::ChildWindow>(mainWindow.get());
        this->mainWindow->addCaption();
        
        childrenWindows.reserve(DEFAULT_WINDOW_COUNT);
        sharedWindowContexts.reserve(DEFAULT_WINDOW_COUNT);
        headerWindows.reserve(DEFAULT_WINDOW_COUNT);
        viewportWindows.reserve(DEFAULT_WINDOW_COUNT);
        
        buildWindows();

        this->mainWindow->show();

        subscribe(
            this->mainWindow.get(), &WindowInterface::IWindow::onSizeChanged,
            [this](const NbSize<int>&)
            {
                this->repositionGrid();
            }
        );
    }

    std::unique_ptr<NNsLayout::LayoutNode> FramebufferVisualization::buildUi() noexcept
    {
        using namespace nbui;
        
        auto ui = LayoutBuilder::hBox()
            .relativeWidth(1.0f)
            .absoluteHeight(static_cast<float>(TOOLBAR_HEIGHT));

        std::wstring titleText = L" ▾ Монитор буферов кадра (" + 
                                 std::to_wstring(childrenWindows.size()) + 
                                 L" каналов)";

        ui = std::move(ui).child(
            LayoutBuilder::widget(new Widgets::ComboBox())
                .relativeHeight(1.0f)
                .absoluteWidth(320)
                .apply<Widgets::ComboBox>([this, titleText](Widgets::ComboBox* c) 
                {
                    //Renderer::RendererTracker::get
                    c->addItem({titleText, 1});
                    c->addItem({L"dasdas", 2});
                    c->addItem({L"dasdasdd", 3});
                    c->addItem({L"dasdasdasdasd", 4});
                })
        );

        ui = std::move(ui).child(
            LayoutBuilder::hBox()
                .relativeWidth(1.0f)
                .relativeHeight(1.0f)
        );

        ui = std::move(ui).child(
            LayoutBuilder::widget(new Widgets::Button())
                .relativeHeight(1.0f)
                .absoluteWidth(90)
                .text(L"Просмотр")
        );

        ui = std::move(ui).child(
            LayoutBuilder::widget(new Widgets::Button())
                .relativeHeight(1.0f)
                .absoluteWidth(100)
                .text(L"Параметры")
        );

        return std::move(ui).build();
    }

    void FramebufferVisualization::repositionGrid() noexcept
    {
        if (childrenWindows.isEmpty())
        {
            return;
        }

        const auto engineRect = mainWindow->getClientRect();
        const int workspaceX = engineRect.x;
        const int workspaceY = engineRect.y;
        const int workspaceW = engineRect.width;
        const int workspaceH = engineRect.height;

        if (toolbarWindow)
        {
            toolbarWindow->setPosition({workspaceX, workspaceY});
            toolbarWindow->setSize({workspaceW, TOOLBAR_HEIGHT});
        }

        const int paddingX = 5;
        const int paddingY = 5;

        const int marginL = workspaceX + paddingX;
        const int marginT = workspaceY + TOOLBAR_HEIGHT + paddingY;

        const size_t total = childrenWindows.size();

        auto positionInnerWindows = [this](size_t index, int width, int height) noexcept {
            if (index < headerWindows.size() && index < viewportWindows.size())
            {
                headerWindows[index]->setPosition({0, 0});
                headerWindows[index]->setSize({width, CELL_HEADER_HEIGHT});

                viewportWindows[index]->setPosition({0, CELL_HEADER_HEIGHT});
                viewportWindows[index]->setSize({width, height - CELL_HEADER_HEIGHT});
            }
        };

        if (total == 1)
        {
            const int x = marginL;
            const int y = marginT;
            const int w = workspaceX + workspaceW - paddingX - x;
            const int h = workspaceY + workspaceH - paddingY - y;

            childrenWindows[0]->setPosition({x, y});
            childrenWindows[0]->setSize({w, h});

            positionInnerWindows(0, w, h);
            return;
        }

        const int totalGridW = workspaceW - paddingX - paddingX;
        const int totalGridH = workspaceH - TOOLBAR_HEIGHT - paddingY - paddingY;

        constexpr float leftRatio = 0.5f;

        const int leftWidth = static_cast<int>((totalGridW - paddingX) * leftRatio);
        const int rightWidth = totalGridW - leftWidth - paddingX;

        {
            const int x = marginL;
            const int y = marginT;
            const int w = leftWidth;
            const int h = totalGridH;
            
            childrenWindows[0]->setPosition({x, y});
            childrenWindows[0]->setSize({w, h});

            positionInnerWindows(0, w, h);
        }

        const size_t rightTotal = total - 1;
        const size_t rightCols = 2;
        const size_t rightRows = (rightTotal + rightCols - 1) / rightCols;

        const int rightCellW = (rightWidth - (static_cast<int>(rightCols) - 1) * paddingX) / static_cast<int>(rightCols);
        const int rightCellH = (totalGridH - (static_cast<int>(rightRows) - 1) * paddingY) / static_cast<int>(rightRows);

        const int rightGridStartX = marginL + leftWidth + paddingX;

        for (size_t i = 1; i < total; ++i)
        {
            const int rightIdx = static_cast<int>(i - 1);
            const int col = rightIdx % static_cast<int>(rightCols);
            const int row = rightIdx / static_cast<int>(rightCols);

            const int x = rightGridStartX + col * (rightCellW + paddingX);
            const int y = marginT + row * (rightCellH + paddingY);

            const int w = (col == static_cast<int>(rightCols) - 1) ? (workspaceX + workspaceW - paddingX - x) : rightCellW;
            const int h = (row == static_cast<int>(rightRows) - 1) ? (workspaceY + workspaceH - paddingY - y) : rightCellH;

            childrenWindows[i]->setPosition({x, y});
            childrenWindows[i]->setSize({w, h});
            
            positionInnerWindows(i, w, h);
        }
    }

    void FramebufferVisualization::buildWindows() noexcept
    {
        childrenWindows.clear();
        sharedWindowContexts.clear();
        headerWindows.clear();
        viewportWindows.clear();

        toolbarWindow = std::make_unique<Win32Window::ChildWindow>(mainWindow.get());
        toolbarWindow->getLayoutRoot()->addChild(buildUi());

        HWND parentHwnd = mainWindow->getHandle().as<HWND>();

        LONG_PTR parentStyle = GetWindowLongPtr(parentHwnd, GWL_STYLE);
        if (!(parentStyle & WS_CLIPCHILDREN))
        {
            SetWindowLongPtr(parentHwnd, GWL_STYLE, parentStyle | WS_CLIPCHILDREN);
        }

        auto frame = nb::Renderer::RendererTracker::getFrameBuffer("GBuffer");
        const size_t textureCount = frame ? frame->getTextureCount() : 0;

        for (size_t i = 0; i < textureCount; i++)
        {
            auto container = std::make_unique<Win32Window::ChildWindow>(mainWindow.get());
            HWND containerHwnd = container->getHandle().as<HWND>();

            LONG_PTR containerStyle = GetWindowLongPtr(containerHwnd, GWL_STYLE);
            SetWindowLongPtr(containerHwnd, GWL_STYLE, containerStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

            auto header = std::make_unique<Win32Window::ChildWindow>(container.get());
            
            std::wstring channelName = Utils::toWstring(frame->getNameByTextureIndex(i));

            std::wstring formatName = L"RGB_8";

            switch (frame->getTextureAttachmentType(i))
            {
                case nb::Renderer::IFrameBuffer::TextureAttachment::COLOR:
                {
                    formatName = L"GL_RGBA8";
                    break;
                }
                case nb::Renderer::IFrameBuffer::TextureAttachment::COLOR_HDR:
                {
                    formatName = L"GL_RGB16F";
                    break;
                }
                case nb::Renderer::IFrameBuffer::TextureAttachment::DEPTH:
                {
                    formatName = L"GL_DEPTH_COMPONENT32";
                    break;
                }
                case nb::Renderer::IFrameBuffer::TextureAttachment::STENCIL:
                {
                    formatName = L"GL_STENCIL_INDEX8";
                    break;
                }
                case nb::Renderer::IFrameBuffer::TextureAttachment::DEPTH_STENCIL:
                {
                    formatName = L"GL_DEPTH24_STENCIL8";
                    break;
                }
            }

            header->getLayoutRoot()->addChild(nbui::LayoutBuilder::hBox().relativeHeight(1.0f).relativeWidth(1.0f)
                .child(
                    nbui::LayoutBuilder::label(channelName).relativeHeight(1.0f).relativeWidth(0.5f).textAlignment({.textAlignment = TextAlignment::LEFT, .gap = 10})
                )
                .child(
                    nbui::LayoutBuilder::label(formatName).relativeHeight(1.0f).relativeWidth(0.5f).textAlignment({.textAlignment = TextAlignment::RIGHT, .gap = 10}).color({80, 80, 80})
                )
                .build());
            auto viewport = std::make_unique<Win32Window::ChildWindow>(container.get(), true);
            HWND viewportHwnd = viewport->getHandle().as<HWND>();

            LONG_PTR viewportStyle = GetWindowLongPtr(viewportHwnd, GWL_STYLE);
            SetWindowLongPtr(viewportHwnd, GWL_STYLE, viewportStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

            childrenWindows.pushBack(std::move(container));
            headerWindows.pushBack(std::move(header));
            viewportWindows.pushBack(std::move(viewport));

            sharedWindowContexts.pushBack(engine->getRenderer()->createSharedContextForWindow(viewportWindows[i]->getHandle().as<HWND>()));
            
            subscribe(viewportWindows[i].get(), &WindowInterface::IWindow::onDraw, [this, i]() 
            {
                auto frame = nb::Renderer::RendererTracker::getFrameBuffer("GBuffer");
                if (frame)
                {
                    engine->getRenderer()->renderFramebufferToContext(sharedWindowContexts[i], frame, i);
                }

                HWND hwnd = viewportWindows[i]->getHandle().as<HWND>();
                InvalidateRect(hwnd, nullptr, FALSE);
            });
        }

        repositionGrid();
    }

    void FramebufferVisualization::addFramebuffer() noexcept
    {

    }

}; // namespace Sdk