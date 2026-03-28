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
std::unique_ptr<NNsLayout::LayoutNode> AssetManager::buildUI()
{
    using namespace nbui;

    return LayoutBuilder::vBox()
        .relativeHeight(1.0f)
        .relativeWidth(1.0f)

        .child(
            LayoutBuilder::toolbar()
                .border(1, Border::Style::SOLID, {251, 251, 251}, Border::Side::BOTTOM)
                .child(
                    LayoutBuilder::widget(new Widgets::Button())
                        .relativeHeight(1.0f)
                        .relativeWidth(0.1f)
                        .text(L"Import")
                )

                .child(LayoutBuilder::spacerAbsolute(10.0f, 1.0f))

                .child(
                    LayoutBuilder::widget(new Widgets::Button())
                        .relativeHeight(1.0f)
                        .relativeWidth(0.1f)
                        .text(L"Add Folder")
                )
        )

        .child(
            LayoutBuilder::hBox()
                .relativeHeight(1.0f)
                .relativeWidth(1.0f)

                .child(
                    LayoutBuilder::vBox()
                        .relativeHeight(1.0f)
                        .relativeWidth(0.3f)
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
                                    [&](auto* tv)
                                    {
                                        tv->setModel(model);
                                    }
                                )
                                .onEvent(
                                    &Widgets::TreeView::onItemChangeSignal,
                                    [&](const Widgets::ModelIndex& index)
                                    {
                                        auto* item = this->model->findById(index.getUuid());
                                        if (item)
                                        {
                                            this->onFolderSelected(this->model->getPath(*item));
                                        }
                                    }
                                )
                                .relativeHeight(1.0f)
                                .relativeWidth(1.0f)
                                .background({28, 28, 28})
                        )
                )

                .child(
                    LayoutBuilder::vBox()
                        .relativeHeight(1.0f)
                        .relativeWidth(0.7f)

                        .apply<NNsLayout::LayoutNode>(
                            [this](NNsLayout::LayoutNode* node)
                            {
                                this->assetGridNode = node;
                            }
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

    auto grid = LayoutBuilder::flow().relativeWidth(1.0f).autoHeight().padding({15, 15, 15, 15});

    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(currentPath))
        {
            if (entry.is_regular_file())
            {
                std::wstring fileName = entry.path().filename().wstring();
                std::wstring ext = entry.path().extension().wstring();

                NbColor accent = getAccentColorForExt(ext);

                

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
                                    if (supportedExtensions[entry.path().extension().string()] ==
                                        AssetType::TEXTURE)
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
    }
    catch (...)
    {
    }

    assetGridNode->addChild(std::move(grid).build());
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
    if (ext == L".hlsl")
    {
        return {156, 39, 176}; // Фиолетовый
    }
    return {150, 150, 150}; // Серый
}
