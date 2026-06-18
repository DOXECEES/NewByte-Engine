#include "AssetManger.hpp"

#include "MaterialEditor.hpp"
#include <Widgets/Thumbnail.hpp>
#include <memory>

#include "App.hpp"

AssetManager::AssetManager(
    std::shared_ptr<Win32Window::ChildWindow> wnd,
    nbstl::NonOwningPtr<nb::Core::Engine> engine
) 
    : engine(engine)
    , window(wnd)
{
    
    treeWindow      = std::make_shared<Win32Window::ChildWindow>(window.get());
    assetGridWindow = std::make_shared<Win32Window::ChildWindow>(window.get());

    handleResize(window->getClientSize());

    treeWindow->getLayoutRoot()->addChild(buildTreeUI());      
    assetGridWindow->getLayoutRoot()->addChild(buildGridUI()); 

    subscribe(
        window.get(), &Win32Window::ChildWindow::onSizeChanged,
        [this](const NbSize<int>& newSize)
        {
            handleResize(newSize);
        }
    );

    window->show();
    treeWindow->show();
    assetGridWindow->show();
}

void AssetManager::handleResize(const NbSize<int>& size)
{
    const int toolbarHeight = 0;
    const int treeWidth     = 250;
    const int totalWidth    = size.width;
    const int totalHeight   = size.height;

    treeWindow->setPosition({0, toolbarHeight});
    treeWindow->setSize({treeWidth, totalHeight - toolbarHeight});

    assetGridWindow->setPosition({treeWidth, toolbarHeight});
    assetGridWindow->setSize({totalWidth - treeWidth, totalHeight - toolbarHeight});
}

void AssetManager::importAsset(std::filesystem::path path) noexcept
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

void AssetManager::rebuildTreeAndPreserveState(Widgets::TreeView* tv)
{
    if (!tv || !model || isTreeUpdating)
    {
        return;
    }

    isTreeUpdating = true;

    std::vector<std::filesystem::path> expandedPaths;
    std::filesystem::path              selectedPath;

    model->forEach(
        [&](const Widgets::ModelItem& item)
        {
            Widgets::ModelIndex index(item.getUuid());

            if (tv->isItemExpanded(index))
            {
                expandedPaths.push_back(model->getPath(item));
            }
            if (tv->isItemSelected(index))
            {
                selectedPath = model->getPath(item);
            }
        }
    );

    model->rebuildModel("Assets");
    tv->refresh();                 

    model->forEach(
        [&](const Widgets::ModelItem& item)
        {
            auto                currentPath = model->getPath(item);
            Widgets::ModelIndex index(item.getUuid());

            auto it = std::find(expandedPaths.begin(), expandedPaths.end(), currentPath);
            if (it != expandedPaths.end())
            {
                tv->setItemExpanded(index, true);
            }

            if (currentPath == selectedPath)
            {
                tv->setSelectedItem(index);
            }
        }
    );

    isTreeUpdating = false;
}

std::unique_ptr<NNsLayout::LayoutNode> AssetManager::buildTreeUI()
{
    using namespace nbui;
    return LayoutBuilder::vBox()
        .relativeWidth(1.0f)
        .relativeHeight(1.0f)
        .background({28, 28, 28})
        //.child(LayoutBuilder::label(L"Folders").relativeWidth(1.0f).absoluteHeight(30.0f))
        .child(
            LayoutBuilder::treeView()
                .relativeWidth(1.0f)
                .relativeHeight(1.0f)
                .apply<Widgets::TreeView>(
                    [&](Widgets::TreeView* tv)
                    {
                        subscribe(
                            tv, &Widgets::TreeView::onItemChangeSignal,
                            [this, tv](const Widgets::ModelIndex& index)
                            {
                                if (isTreeUpdating)
                                {
                                    return; 
                                } 


                                auto* item = this->model->findById(index.getUuid());
                                if (item)
                                {
                                    this->onFolderSelected(this->model->getPath(*item));
                                    rebuildTreeAndPreserveState(tv);
                                }
                            }
                        );

                        treeView = tv;

                        tv->setModel(model);
                    }
                )
        )
        .build();
}

std::unique_ptr<NNsLayout::LayoutNode> AssetManager::buildGridUI()
{
    using namespace nbui;
    return LayoutBuilder::scrollBox()
        .relativeWidth(1.0f)
        .relativeHeight(1.0f)
        .child(
            LayoutBuilder::vBox().relativeWidth(1.0f).autoHeight().apply<NNsLayout::LayoutNode>(
                [this](auto* n)
                {
                    this->assetGridNode = n; 
                }
            )
        )
        .build();
}

std::unique_ptr<NNsLayout::LayoutNode> AssetManager::buildUI()
{
    using namespace nbui;

    return LayoutBuilder::vBox()
        .relativeHeight(1.0f)
        .relativeWidth(1.0f)

        // .child(
        //     LayoutBuilder::toolbar()
        //         .style(
        //             [](NNsLayout::LayoutStyle& s)
        //             {
        //                 s.color = {40, 40, 40};
        //                 s.heightSizeType = NNsLayout::SizeType::ABSOLUTE;
        //                 s.height = 35.0f;
        //             }
        //         )
        //         .child(
        //             LayoutBuilder::widget(new Widgets::Button())
        //                 .text(L"  Import  ") 
        //                 .absoluteWidth(80)
        //                 .margin({5, 5, 5, 5})
        //                 .background({60, 60, 60})
        //         )
        //         .child(
        //             LayoutBuilder::widget(new Widgets::Button())
        //                 .text(L"  Add Folder  ")
        //                 .absoluteWidth(100)
        //                 .margin({0, 5, 5, 5})
        //                 .background({60, 60, 60})
        //         )

        // )

        .child(
            LayoutBuilder::hBox()
                .relativeHeight(1.0f)
                .relativeWidth(1.0f)

                .child(
                    LayoutBuilder::vBox()
                        .relativeHeight(1.0f)
                        .absoluteWidth(250.0f)
                        .style(
                            [](NNsLayout::LayoutStyle& s)
                            {
                                s.border.style = Border::Style::SOLID;
                                s.border.width.right = 1;
                                s.border.color = {251, 251, 251};
                                s.border.sideMask = Border::Side::RIGHT;
                            }
                        )

                        .child(
                            LayoutBuilder::label(L"Folders")
                                .relativeWidth(1.0f)
                                .absoluteHeight(30.0f)
                        )

                        .child(
                            LayoutBuilder::treeView()
                                .apply<Widgets::TreeView>(
                                    [&](Widgets::TreeView* tv)
                                    {
                                        subscribe(tv, &Widgets::TreeView::onItemChangeSignal,
                                            [&](const Widgets::ModelIndex& index)
                                            {
                                                auto* item = this->model->findById(index.getUuid());
                                                if (item)
                                                {
                                                    this->onFolderSelected(this->model->getPath(*item));
                                                    model->rebuildModel("Assets");
                                                    tv->refresh();
                                                }
                                            });

                                        tv->setModel(model);
                                    }
                                )
                                .relativeHeight(1.0f)
                                .relativeWidth(1.0f)
                                .background({28, 28, 28})
                        )
                )

                .child(
                        LayoutBuilder::vBox()
                            .absoluteHeight(500.0f)
                            .relativeWidth(1.0f) 
                            .child(
                                LayoutBuilder::scrollBox()
                                    .relativeHeight(1.0f)
                                    .relativeWidth(1.0f) 
                                    .child(
                                        LayoutBuilder::vBox()
                                            .relativeWidth(1.0f) 
                                            .relativeHeight(1.0f)
                                            .apply<NNsLayout::LayoutNode>(
                                                [this](auto* n)
                                                {
                                                    this->assetGridNode = n;
                                                }
                                            )
                                    )
                            )
                    )
        )

        .build();
}

void AssetManager::onFolderSelected(std::filesystem::path path)
{
    this->currentPath = path;
    this->refreshAssetGrid();
}

void AssetManager::refreshAssetGrid()
{
    if (!assetGridNode)
    {
        return;
    }

    using namespace nbui;
    assetGridNode->clearChilds();

    auto grid = LayoutBuilder::flow().relativeWidth(1.0f).autoHeight().padding({20, 20, 20, 20});

    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(currentPath))
        {
            if(entry.is_directory())
            {
                createFolderThumbnail(grid, entry.path());
                continue;
            }
        }

        for (const auto& entry : std::filesystem::directory_iterator(currentPath))
        {
            if (!entry.is_regular_file())
            {
                continue;
            }

            std::string extStr = entry.path().extension().string();
            if (!supportedExtensions.contains(extStr))
            {
                continue;
            }

            std::wstring fullFileName = entry.path().filename().wstring();
            std::wstring displayName  = entry.path().stem().wstring();
            std::wstring extension    = entry.path().extension().wstring();
            NbColor      accentColor  = getAccentColorForExt(extension);

            if (supportedExtensions.at(extStr) == Widgets::AssetType::MATERIAL)
            {
                std::string replacedPath = entry.path().generic_string();
                std::replace(replacedPath.begin(), replacedPath.end(), '/', '_');
                if (!std::filesystem::exists("Assets/cache/" + replacedPath + ".png"))
                {
                    nb::Renderer::Renderer::generatePreviewForMaterial(
                        entry.path().generic_string()
                    );
                }
            }
            

            std::move(grid).child(
                LayoutBuilder::vBox()
                    .margin({0, 12, 12, 0}) 
                    .style(
                        [accentColor](NNsLayout::LayoutStyle& s)
                        {
                            s.width  = 120;
                            s.height = 160; 
                            s.color  = {38, 38, 38};
                            s.border.radius = 4.0f;

                            s.border.style        = Border::Style::SOLID;
                            s.border.width.bottom = 3;
                            s.border.color        = accentColor;
                            s.border.sideMask     = Border::Side::BOTTOM;
                        }
                    )
                    .child(
                        LayoutBuilder::thumbnail(
                            fullFileName, L"", supportedExtensions.at(extStr), entry.path()
                        )
                            .relativeWidth(1.0f)
                            .absoluteHeight(95)
                            .background({25, 25, 25})
                            .apply<Widgets::IWidget>(
                                [this, entry](Widgets::IWidget* w)
                                {
                                    w->onPressedSignal.connect(
                                        [this, entry]()
                                        {
                                            this->dragInfo.active = true;
                                            this->dragInfo.path   = entry.path();
                                            GetCursorPos(&this->dragInfo.startMousePos);
                                            SetCapture((HWND)this->window->getHandle().as<HWND>());
                                        }
                                    );

                                    w->onReleasedSignal.connect(
                                        [this, entry]()
                                        {
                                            if (!this->dragInfo.active)
                                            {
                                                return;
                                            }

                                            ReleaseCapture();
                                            this->dragInfo.active = false;

                                            POINT pt;
                                            GetCursorPos(&pt);
                                            int   dx = pt.x - this->dragInfo.startMousePos.x;
                                            int   dy = pt.y - this->dragInfo.startMousePos.y;
                                            float distance =
                                                std::sqrt(static_cast<float>(dx * dx + dy * dy));

                                            if (distance > this->dragThreshold)
                                            {
                                                HWND target = WindowFromPoint(pt);
                                                HWND glHWnd = (HWND)engine->getLinkedHwnd();
                                                if (target == glHWnd)
                                                {
                                                    ScreenToClient(glHWnd, &pt);
                                                    if (dragInfo.path.extension() == ".model")
                                                    {
                                                        auto spawnPos = this->engine->getSpawnPosition(
                                                            pt.x, pt.y
                                                        );

                                                        EditorApp::requestModelSpawn(
                                                            {
                                                                .position    = spawnPos,
                                                                .pathToModel = dragInfo.path,
                                                            }
                                                        );

                                                        
                                                    }
                                                    else
                                                    {
                                                        this->engine->getRenderer()
                                                            ->pickNodeAndApplyMaterial(
                                                                pt.x, pt.y, this->dragInfo.path
                                                            );
                                                    }
                                                    
                                                }
                                            }
                                            else
                                            {
                                                //if(supportedExtensions.at(ext) )

                                                if (this->textureEditor)
                                                {
                                                    this->textureEditor = nullptr;
                                                }


                                                auto textureRes =
                                                    nb::ResMan::ResourceManager::getInstance()
                                                        ->getResource<nb::Resource::TextureAsset>(
                                                            "Assets/res/" + entry.path().stem().string() +
                                                            ".texture"
                                                        );

                                                if (textureRes)
                                                {
                                                    this->textureEditor =
                                                        std::make_shared<TextureEditor>(
                                                            this->window.get(), this->engine.get(),
                                                            textureRes.get()
                                                        );
                                                    this->textureEditor->show();
                                                }
                                            }
                                        }
                                    );
                                }
                            )
                    )
                    .child(
                        LayoutBuilder::vBox()
                            .style(
                                [](auto& s)
                                {
                                    s.padding = {8, 6, 8, 4};
                                }
                            )
                            .child(
                                LayoutBuilder::label(displayName)
                                    .fontSize(10)
                                    .color({230, 230, 230})
                                    .absoluteHeight(28)
                                    .apply<Widgets::Label>(
                                        [](Widgets::Label* l)
                                        {
                                            l->setEllipsis(
                                                true
                                            ); 
                                        }
                                    )
                            )
                            .child(
                                LayoutBuilder::label(extension)
                                    .fontSize(9)
                                    .color({110, 110, 110})
                                    .absoluteHeight(20)
                            )
                    )
                    
            );
        }
    }
    catch (...)
    {
    }

    assetGridNode->addChild(std::move(grid).build());
}

void AssetManager::refreshModel() noexcept
{
    if (treeView)
    {
        rebuildTreeAndPreserveState(treeView);
    }
}

NbColor AssetManager::getAccentColorForExt(const std::wstring& ext)
{
    if (ext == L".png" || ext == L".tga")
    {
        return {76, 175, 80}; // Зеленый
    }
    if (ext == L".obj" || ext == L".fbx")
    {
        return {33, 150, 243}; // Синий
    }
    if (ext == L".hlsl" || ext == L".lua")
    {
        return {156, 39, 176}; // Фиолетовый
    }
    return {150, 150, 150}; // Серый
}

void AssetManager::createFolderThumbnail(nbui::LayoutBuilder& grid, const std::filesystem::path& path)
{
    using namespace nbui;

    std::wstring directoryName = path.stem().wstring();

    std::move(grid).child(
        LayoutBuilder::vBox()
            .margin({0, 12, 12, 0})
            .style(
                [](NNsLayout::LayoutStyle& s)
                {
                    s.width         = 120;
                    s.height        = 160;
                    s.color         = {38, 38, 38};
                    s.border.radius = 4.0f;

                    s.border.style        = Border::Style::SOLID;
                    s.border.width.bottom = 3;
                    s.border.color        = {255, 255, 255};
                    s.border.sideMask     = Border::Side::BOTTOM;
                }
            )
            .child(
                LayoutBuilder::thumbnail(
                    directoryName, L"", Widgets::AssetType::FOLDER,  path
                )
                    .relativeWidth(1.0f)
                    .absoluteHeight(95)
                    .background({25, 25, 25})
                    .apply<Widgets::IWidget>(
                        [this, path](Widgets::IWidget* w) 
                        {
                            w->onReleasedSignal.connect(
                                [this, path]() 
                                {
                                    bool itemFound = false;
                                    Widgets::ModelIndex targetIndex;

                                    if (this->model && this->treeView)
                                    {
                                        this->model->forEach(
                                            [&](const Widgets::ModelItem& item)
                                            {
                                                if (this->model->getPath(item) == path)
                                                {
                                                    targetIndex = Widgets::ModelIndex(item.getUuid());
                                                    itemFound = true;
                                                }
                                            }
                                        );
                                    }

                                    if (itemFound)
                                    {
                                        this->treeView->setSelectedItem(targetIndex);
                                    }
                                }
                            );
                        }
                    )
            )
            .child(
                LayoutBuilder::vBox()
                    .style(
                        [](auto& s)
                        {
                            s.padding = {8, 6, 8, 4};
                        }
                    )
                    .child(
                        LayoutBuilder::label(directoryName)
                            .fontSize(10)
                            .color({230, 230, 230})
                            .absoluteHeight(28)
                            .apply<Widgets::Label>(
                                [](Widgets::Label* l)
                                {
                                    l->setEllipsis(true);
                                }
                            )
                    )
                    .child(
                        LayoutBuilder::label(L"Folder")
                            .fontSize(9)
                            .color({110, 110, 110})
                            .absoluteHeight(20)
                    )
            )

    );
}
