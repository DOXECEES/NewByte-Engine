#ifndef SDK_IMPORTWINDOW_HPP
#define SDK_IMPORTWINDOW_HPP

#include <Core/Engine.hpp>
#include <NonOwningPtr.hpp>

#include <Win32Window/Win32ChildWindow.hpp>

#include <LayoutBuilder.hpp>
#include <Widgets/Button.hpp>
#include <Widgets/ComboBox.hpp>
#include <Widgets/Slider.hpp>

#include <filesystem>
#include <memory>
#include <string>

#include <Manager/AssetImporter.hpp>

class ImportWindow
{
public:
    struct ImportSettings
    {
        std::filesystem::path sourcePath;
        std::filesystem::path targetFolder = "Assets/res";

        std::wstring assetName;

        bool importMeshes    = true;
        bool importMaterials = true;
        bool importTextures  = true;

        bool generateTangents = true;
        bool flipUV           = false;

        bool copyTexturesIntoProject = true;

        enum class NormalConvention
        {
            OpenGL,
            DirectX
        } normalConvention = NormalConvention::OpenGL;

        enum class TextureCompression
        {
            None,
            BC3,
            BC7
        } textureCompression = TextureCompression::BC7;
    };

public:
    ImportWindow(
        std::shared_ptr<Win32Window::ChildWindow> wnd,
        nbstl::NonOwningPtr<nb::Core::Engine>     engine,
        std::filesystem::path                     sourcePath
    );

    ~ImportWindow() = default;

    std::unique_ptr<NNsLayout::LayoutNode> buildUI();
    nbui::LayoutBuilder                    createOptionToggle(
        const std::wstring&   name,
        bool&                 value,
        std::function<void()> onClick
    );

private:
    void importAsset();
    void close();

    void updateAssetNameFromPath();

private:
    std::shared_ptr<Win32Window::ChildWindow> window;
    nbstl::NonOwningPtr<nb::Core::Engine>     engine;
    std::unique_ptr<nb::SDK::AssetImporter>   importer;

    ImportSettings settings;
};

#endif