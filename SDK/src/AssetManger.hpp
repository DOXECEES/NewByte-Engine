#ifndef SDK_ASSETMANAGER_HPP
#define SDK_ASSETMANAGER_HPP

#include "Core.hpp"

#include <NonOwningPtr.hpp>
#include <Core/Engine.hpp>

#include <string>
#include "Loaders/JSON/Json.hpp"
#include "Widgets/TreeView.hpp"
#include <Win32Window/Win32Window.hpp>

#include <Win32Window/Win32ChildWindow.hpp>

#include <LayoutBuilder.hpp>
#include <Widgets/Button.hpp>
#include <Widgets/Slider.hpp>
#include <Widgets/ComboBox.hpp>

#include <memory>
#include <filesystem>

#include <Manager/ResourceManager.hpp>
#include <Renderer/Shader.hpp>
#include <Renderer/Mesh.hpp>
#include "FileSystemModel.hpp"
#include "Widgets/WidgetStyle.hpp"

#include <Loaders/JSON/Json.hpp>

#include "TextureEditor.hpp"
#include "MaterialEditor.hpp"
#include <Widgets/Thumbnail.hpp>

class AssetManager
{
public:
    
    AssetManager(
        std::shared_ptr<Win32Window::ChildWindow> wnd,
        nbstl::NonOwningPtr<nb::Core::Engine>     engine
    );
    ~AssetManager() = default;
    void handleResize(const NbSize<int>& size);

    void importAsset(std::filesystem::path path) noexcept;
    void rebuildTreeAndPreserveState(Widgets::TreeView* tv);

    std::unique_ptr<NNsLayout::LayoutNode> buildTreeUI();
    std::unique_ptr<NNsLayout::LayoutNode> buildGridUI();

    void onFolderSelected(std::filesystem::path path);

    void refreshAssetGrid();
    void refreshModel() noexcept;

    NbColor getAccentColorForExt(const std::wstring& ext);

    std::shared_ptr<Win32Window::ChildWindow> getWindow() const noexcept
    {
        return window;
    }

    std::shared_ptr<Win32Window::ChildWindow> getAssetGridWindow() const noexcept
    {
        return assetGridWindow;
    }
    //Signal<void()>

private:
    void createFolderThumbnail(nbui::LayoutBuilder& grid, const std::filesystem::path& path, bool isReturnFolder = false);


    struct DragInfo
    {
        bool                  active            = false;
        bool                  isDraggingStarted = false; 
        POINT                 startMousePos;             
        std::filesystem::path path;                      
    } dragInfo;

    const int dragThreshold = 5; 

    std::unordered_map<std::string, Widgets::AssetType> supportedExtensions = {
        {".png", Widgets::AssetType::TEXTURE},
        {".material", Widgets::AssetType::MATERIAL}, 
        {".model", Widgets::AssetType::MODEL},
        {".lua", Widgets::AssetType::SCRIPT},
        {".vs", Widgets::AssetType::SHADER},
        {".fs", Widgets::AssetType::SHADER},
        {".translation", Widgets::AssetType::TRANSLATION},
        {"folder", Widgets::AssetType::FOLDER}
        //{".jpg", AssetType::TEXTURE},
        //{".texture", AssetType::TEXTURE},
        //{".fbx", AssetType::MODEL},
        //{".glsl", AssetType::SHADER}
    };
     

    NNsLayout::LayoutNode* assetGridNode = nullptr;
    std::shared_ptr<FileSystemModel> model = std::make_shared<FileSystemModel>("Assets");
    std::filesystem::path currentPath;


    std::shared_ptr<Win32Window::ChildWindow> window;
    std::shared_ptr<Win32Window::ChildWindow> treeWindow;
    std::shared_ptr<Win32Window::ChildWindow> assetGridWindow;


    nbstl::NonOwningPtr<nb::Core::Engine> engine;
    std::shared_ptr<TextureEditor>        textureEditor;
    std::shared_ptr<MaterialEditor>       materialEditor;

    Widgets::TreeView*                    treeView;
    bool                                  isTreeUpdating = false;

    inline static nb::Loaders::Json assetsJson = nb::Loaders::Json(std::filesystem::path("Assets/Assets.json"));
};

#endif