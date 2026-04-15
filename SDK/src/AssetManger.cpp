#include "AssetManger.hpp"

#include "MaterialEditor.hpp"
#include <memory>

AssetManager::AssetManager(
    std::shared_ptr<Win32Window::ChildWindow> wnd,
    nbstl::NonOwningPtr<nb::Core::Engine> engine
) 
    : engine(engine)
    , window(wnd)
{
    

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
                .style(
                    [](NNsLayout::LayoutStyle& s)
                    {
                        s.color = {40, 40, 40};
                        s.heightSizeType = NNsLayout::SizeType::ABSOLUTE;
                        s.height = 35.0f;
                    }
                )
                .child(
                    LayoutBuilder::widget(new Widgets::Button())
                        .text(L"  Import  ") 
                        .absoluteWidth(80)
                        .margin({5, 5, 5, 5})
                        .background({60, 60, 60})
                )
                .child(
                    LayoutBuilder::widget(new Widgets::Button())
                        .text(L"  Add Folder  ")
                        .absoluteWidth(100)
                        .margin({0, 5, 5, 5})
                        .background({60, 60, 60})
                )

        )

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
                        .relativeWidth(1.0f)

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

    // Сетка с хорошим внешним отступом
    auto grid = LayoutBuilder::flow().relativeWidth(1.0f).autoHeight().padding({20, 20, 20, 20});

    try
    {
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
                        LayoutBuilder::thumbnail(fullFileName, L"")
                            .relativeWidth(1.0f)
                            .absoluteHeight(95)
                            .background({25, 25, 25})
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
                                    int dx = pt.x - this->dragInfo.startMousePos.x;
                                    int dy = pt.y - this->dragInfo.startMousePos.y;

                                    if (std::sqrt(dx * dx + dy * dy) > this->dragThreshold)
                                    {
                                        HWND target = WindowFromPoint(pt);
                                        HWND glHWnd = (HWND)engine->getLinkedHwnd();
                                        if (target == glHWnd)
                                        {
                                            ScreenToClient(glHWnd, &pt);
                                            this->engine->getRenderer()->pickNodeAndApplyMaterial(
                                                pt.x, pt.y, this->dragInfo.path
                                            );
                                        }
                                    }
                                }
                            );
                        }
                    )
            );
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
