#ifndef SDK_TEXTUREEDITOR_HPP
#define SDK_TEXTUREEDITOR_HPP

#include <Core.hpp>
#include <LayoutBuilder.hpp>
#include <Layout/LayoutNode.hpp>

#include <Win32Window/Win32ChildWindow.hpp>
#include <Win32Window/Win32ModalWindow.hpp>

#include <Core/Engine.hpp>
#include <Renderer/Renderer.hpp>

#include <memory>

#include <NonOwningPtr.hpp>

#include <Resources/TextureAsset.hpp>

//#include <NonOwningPtr.hpp>

class TextureEditor
{
public:
    TextureEditor(
        WindowInterface::IWindow* parent,
        nb::Core::Engine* engine,
        nbstl::NonOwningPtr<nb::Resource::TextureAsset> texture
    );

    ~TextureEditor();

    void show();
    void setVisible(bool visible);

    void setTargetTexture(nb::Renderer::Texture* tex);

    void onRender();

    void handleResize(const NbSize<int>& parentSize);

    Win32Window::ModalWindow* getRawWindow()
    {
        return textureEditorWindow.get();
    }

public:
    Signal<void()> onClose;

private:
    std::unique_ptr<NNsLayout::LayoutNode> buildInspectorUI();
    std::unique_ptr<NNsLayout::LayoutNode> buildEditorUI();

private:

    struct Metrics
    {
        static constexpr int windowWidth = 1100;
        static constexpr int windowHeight = 700;
        static constexpr int toolbarHeight = 32;
        static constexpr int titleBarHeight = 35;
        static constexpr int padding = 5;
        static constexpr float previewRatio = 0.7f;
    };


    nb::Core::Engine* engine;
    //nbstl::NonOwningPtr<nb::Core::Engine> engine;
    
    std::shared_ptr<Win32Window::ChildWindow> previewWindow;
    std::shared_ptr<Win32Window::ChildWindow> inspectorWindow;
    std::shared_ptr<Win32Window::ModalWindow> textureEditorWindow;

    nb::Renderer::Renderer::TexturePreviewRequest settings;
    nbstl::NonOwningPtr<nb::Resource::TextureAsset> targetTexture;
    
    nb::Renderer::SharedWindowContext sharedContext;
};

#endif