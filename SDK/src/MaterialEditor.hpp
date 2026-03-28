#ifndef SDK_MATERIALEDITOR_HPP
#define SDK_MATERIALEDITOR_HPP

#include <Core.hpp>
#include <LayoutBuilder.hpp>
#include <Win32Window/Win32ChildWindow.hpp>
#include <Win32Window/Win32ModalWindow.hpp>
#include <Core/Engine.hpp>
#include <Renderer/Renderer.hpp>
#include <Resources/MaterialAsset.hpp> 
#include <memory>
#include <NonOwningPtr.hpp>

class MaterialEditor
{
public:
    MaterialEditor(
        WindowInterface::IWindow* parent,
        nb::Core::Engine* engine,
        nbstl::NonOwningPtr<nb::Resource::MaterialAsset> material
    );

    ~MaterialEditor();

    void show();
    void onRender();
    void handleResize(const NbSize<int>& parentSize);

    Win32Window::ModalWindow* getRawWindow() { return modalWindow.get(); }

private:
    std::unique_ptr<NNsLayout::LayoutNode> buildInspectorUI();
    //std::unique_ptr<NNsLayout::LayoutNode> buildEditorUI();
    
    // Вспомогательная функция для создания виджета под тип параметра
    void addPropertyWidget(nbui::LayoutBuilder&& container, const std::string& name, nb::Resource::MaterialProperty& prop);

private:
    struct Metrics
    {
        static constexpr int toolbarHeight = 32;
        static constexpr int titleBarHeight = 35;
        static constexpr int padding = 5;
        static constexpr float previewRatio = 0.65f;
    };

    nb::Core::Engine* engine;
    std::shared_ptr<Win32Window::ChildWindow> previewWindow;
    std::shared_ptr<Win32Window::ChildWindow> inspectorWindow;
    std::shared_ptr<Win32Window::ModalWindow> modalWindow;

    nbstl::NonOwningPtr<nb::Resource::MaterialAsset> targetMaterial;
    nb::Renderer::SharedWindowContext sharedContext;

    nb::Renderer::Renderer::MaterialPreviewRequest request;
};

#endif