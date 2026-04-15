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

class AssetManager
{
public:
    
    AssetManager(
        std::shared_ptr<Win32Window::ChildWindow> wnd,
        nbstl::NonOwningPtr<nb::Core::Engine>     engine
    );
    ~AssetManager() = default;

    void importAsset(std::filesystem::path path) noexcept;

    std::unique_ptr<NNsLayout::LayoutNode> buildUI();

    void onFolderSelected(std::filesystem::path path);

    void refreshAssetGrid();

    NbColor getAccentColorForExt(const std::wstring& ext);

    std::shared_ptr<Win32Window::ChildWindow> getWindow() const noexcept
    {
        return window;
    }

private:

    struct DragInfo
    {
        bool                  active            = false;
        bool                  isDraggingStarted = false; // Флаг, что это именно перенос, а не клик
        POINT                 startMousePos;             // Где нажали
        std::filesystem::path path;                      // Что тащим
    } dragInfo;

    const int dragThreshold = 5; // Пикселей, после которых клик превращается в драг


    enum class AssetType
    {
        TEXTURE,
        MODEL,
        SHADER,
        MATERIAL,
    };

    std::unordered_map<std::string, AssetType> supportedExtensions = {
        {".png", AssetType::TEXTURE},
        {".material", AssetType::MATERIAL}
        //{".jpg", AssetType::TEXTURE},
        //{".texture", AssetType::TEXTURE},
        //{".fbx", AssetType::MODEL},
        //{".glsl", AssetType::SHADER}
    };
     

    NNsLayout::LayoutNode* assetGridNode = nullptr;
    std::shared_ptr<FileSystemModel> model = std::make_shared<FileSystemModel>("Assets");
    std::filesystem::path currentPath;


    std::shared_ptr<Win32Window::ChildWindow> window;
    nbstl::NonOwningPtr<nb::Core::Engine> engine;

    inline static nb::Loaders::Json assetsJson = nb::Loaders::Json(std::filesystem::path("Assets/Assets.json"));
};

#endif