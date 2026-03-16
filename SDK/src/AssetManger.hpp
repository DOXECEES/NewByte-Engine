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
    
    AssetManager(nbstl::NonOwningPtr<nb::Core::Engine> engine);
    ~AssetManager() = default;

    void importAsset(std::filesystem::path path) noexcept
    {
        if (path.extension() == ".png")
        {
            path.replace_extension(".texture");
        }
        nb::ResMan::ResourceManager* rm = nb::ResMan::ResourceManager::getInstance();
        rm->loadIfNotExists(path);

        assetsJson["NEW_ASSET"]["PATH"] = path.string();
        assetsJson["NEW_ASSET"]["Type"] = "TEXTURE";

        assetsJson.writeToFile("Assets/Assets.json");
    }


    std::unique_ptr<NNsLayout::LayoutNode> buildUI()
    {
        using namespace nbui;

        return LayoutBuilder::vBox()
            .relativeHeight(1.0f)
            .relativeWidth(1.0f)
            
            .child(LayoutBuilder::toolbar()
                .border(1, Border::Style::SOLID, {251, 251, 251}, Border::Side::BOTTOM)
                .child(LayoutBuilder::widget(new Widgets::Button())
                    .relativeHeight(1.0f)
                    .relativeWidth(0.1f)
                    .text(L"Import")
                )

                .child(LayoutBuilder::spacerAbsolute(10.0f, 1.0f))

                .child(LayoutBuilder::widget(new Widgets::Button())
                    .relativeHeight(1.0f)
                    .relativeWidth(0.1f)
                    .text(L"Add Folder")
                )
            )

            .child(LayoutBuilder::hBox()
                .relativeHeight(1.0f)
                .relativeWidth(1.0f)

                .child(LayoutBuilder::vBox()
                    .relativeHeight(1.0f)
                    .relativeWidth(0.3f)
                    .style([](NNsLayout::LayoutStyle& s){
                        s.border.style = Border::Style::SOLID;
                        s.border.width.right = 1;
                        s.border.color = {251, 251, 251};
                        s.border.sideMask = Border::Side::RIGHT;
                    })
                    
                    .child(LayoutBuilder::label(L"Folders")
                        .relativeWidth(1.0f)
                        .absoluteHeight(30.0f)
                    )
                    
                    .child(
                        LayoutBuilder::treeView()
                            .apply<Widgets::TreeView>([&](auto* tv)
                                { 
                                    tv->setModel(model);
                                }
                            )
                            .onEvent(&Widgets::TreeView::onItemChangeSignal, [&](const Widgets::ModelIndex& index){
                                auto* item = this->model->findById(index.getUuid());
                                if (item)
                                {
                                    this->onFolderSelected(this->model->getPath(*item));
                                }

                            })
                            .relativeHeight(1.0f)
                            .relativeWidth(1.0f)
                            .background({28,28,28})
                    )
                )
            
                .child(LayoutBuilder::vBox()
                    .relativeHeight(1.0f)
                    .relativeWidth(0.7f)

                    .apply<NNsLayout::LayoutNode>([this](NNsLayout::LayoutNode* node) 
                    {
                        this->assetGridNode = node; 
                    })
                )
            )
        
        .build();
            

   
    }

    void onFolderSelected(std::filesystem::path path)
    {
        this->currentPath = path;
        this->refreshAssetGrid();
    }

    void refreshAssetGrid() {
        if (!assetGridNode) return;

        using namespace nbui;
        
        // 1. Очищаем текущую сетку
        assetGridNode->clearChilds();

        // 2. Создаем новый FlowLayout (сетку)
        auto grid = LayoutBuilder::flow()
            .relativeWidth(1.0f)
            .autoHeight()
            .padding({15, 15, 15, 15});

        // 3. Сканируем файлы в выбранной папке
        try {
            for (const auto& entry : std::filesystem::directory_iterator(currentPath)) {
                if (entry.is_regular_file()) {
                    std::wstring fileName = entry.path().filename().wstring();
                    std::wstring ext = entry.path().extension().wstring();
                    
                    // Фильтруем типы (можно добавить иконки на основе расширения)
                    NbColor accent = getAccentColorForExt(ext);
                    
                    // std::move(grid).child(
                    //     LayoutBuilder::vBox()
                    //         .absoluteWidth(110)
                    //         .absoluteHeight(140)
                    //         .background(accent)
                    //         .margin({0, 10, 10, 0})
                    //         //.style([accent](auto& s) { s.borderBottom = {3, accent}; s.borderRadius = 4; })
                    //         .child(
                    //             LayoutBuilder::widget(new Widgets::Button())
                    //             .absoluteHeight(90)
                    //             .absoluteWidth(90)
                    //             .background(accent)
                    //         ) // Превью
                    //         .child(LayoutBuilder::vBox()
                    //             .padding({5,5,5,5})
                                 
                    //             .child(LayoutBuilder::label(fileName).fontSize(32)

                    //                 .absoluteHeight(15)
                    //                 .absoluteWidth(90)
                    //                     )
                    //                 .child(
                    //                     LayoutBuilder::label(ext)
                    //                         .fontSize(8)
                    //                         .absoluteHeight(15)
                    //                         .absoluteWidth(90)
                    //                         .color({120, 120, 120})
                    //                 )
                    //         )
                    //         //.onEvent(&Widgets::Button::onClickedSignal, [this, entry]() {
                    //             // Клик по ассету -> открыть в редакторе (например, в TextureEditor)
                    //             //this->openAsset(entry.path());
                    //         //})
                    // );

                    if (supportedExtensions.contains(entry.path().extension().string()))
                    {
                        std::move(grid).child(
                            LayoutBuilder::thumbnail(fileName, ext)
                                .margin({0, 10, 10, 0})
                                .background(accent)
                                .absoluteWidth(120)
                                .absoluteHeight(150)
                                .onEvent(
                                    &Widgets::IWidget::onReleasedSignal,
                                    [this, entry]()
                                    {
                                        if (supportedExtensions[entry.path().extension().string()] == AssetType::TEXTURE)
                                        {
                                            auto copyPath = entry.path();
                                            copyPath = copyPath.replace_extension(".texture");
                                            auto asset = nb::ResMan::ResourceManager::getInstance()
                                                ->getResource<nb::Resource::TextureAsset>(
                                                    copyPath.string()
                                                );

                                            if (asset)
                                            {
                                                auto e = std::make_shared<TextureEditor>(
                                                    window.get(), engine.get(), asset.get()
                                                );

                                                
                                                subscribe(
                                                    e->getRawWindow(), 
                                                    &Win32Window::ModalWindow::onClose,
                                                    [e]()
                                                    {
                                                        auto o = e.use_count();
                                                    }
                                                );
                                                
                                                e->show();
                                            }

                                        }
                                        
                                    }
                                )
                        );
                    }
                }
            }
        } catch (...) {}

        assetGridNode->addChild(std::move(grid).build());
    }

    NbColor getAccentColorForExt(const std::wstring& ext) {
        if (ext == L".png" || ext == L".tga") return {76, 175, 80};  // Зеленый
        if (ext == L".obj" || ext == L".fbx") return {33, 150, 243}; // Синий
        if (ext == L".hlsl") return {156, 39, 176};                 // Фиолетовый
        return {150, 150, 150};                                     // Серый
    }


     

private:

    enum class AssetType
    {
        TEXTURE,
        MODEL,
        SHADER
    };

    std::unordered_map<std::string, AssetType> supportedExtensions = {
        {".png", AssetType::TEXTURE}
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