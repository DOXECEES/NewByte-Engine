#include "ImportWindow.hpp"

#include <Widgets/Button.hpp>
#include <Widgets/ComboBox.hpp>
#include <Widgets/Label.hpp>
#include <Widgets/TextEdit.hpp>
#include <Widgets/CheckBox.hpp>
#include <cwctype>

ImportWindow::ImportWindow(
    std::shared_ptr<Win32Window::ChildWindow> wnd,
    nbstl::NonOwningPtr<nb::Core::Engine>     engine,
    std::filesystem::path                     sourcePath
)
    : window(wnd)
    , engine(engine)
{
    importer = std::make_unique<nb::SDK::AssetImporter>();

    settings.sourcePath = sourcePath;
    updateAssetNameFromPath();

    window->getLayoutRoot()->addChild(buildUI());
    window->show();
}

void ImportWindow::updateAssetNameFromPath()
{
    if (!settings.sourcePath.empty())
    {
        settings.assetName = settings.sourcePath.stem().wstring();
    }
    else
    {
        settings.assetName = L"NewAsset";
    }
}

void ImportWindow::close()
{
    if (window)
    {
        window->close();
    }
}

void ImportWindow::importAsset()
{
    if (!engine || !importer)
    {
        return;
    }

    nb::SDK::ImportRequest request;

    //request.sourceFile   = settings.sourcePath;
    //request.targetFolder = settings.targetFolder;
    //request.assetName    = settings.assetName;

    request.sourceFile = L"Assets/res/scene.gltf";
    request.targetFolder = L"Assets/Models";
    request.assetName    = L"palace";


    request.settings.importMeshes    = settings.importMeshes;
    request.settings.importMaterials = settings.importMaterials;
    request.settings.importTextures  = settings.importTextures;

    request.settings.generateTangents        = settings.generateTangents;
    request.settings.flipUV                  = settings.flipUV;
    request.settings.copyTexturesIntoProject = settings.copyTexturesIntoProject;

    //request.settings.normalConvention =
    //    (settings.normalConvention == ImportSettings::NormalConvention::OpenGL)
    //        ? nb::SDK::ImportSettings::NormalConvention::OpenGL
    //        : nb::SDK::ImportSettings::NormalConvention::DirectX;

    //request.settings.compression =
    //    (settings.textureCompression == ImportSettings::TextureCompression::None)
    //        ? nb::SDK::ImportSettings::TextureCompression::None
    //    : (settings.textureCompression == ImportSettings::TextureCompression::BC3)
    //        ? nb::SDK::ImportSettings::TextureCompression::BC3
    //        : nb::SDK::ImportSettings::TextureCompression::BC7;

    auto result = importer->import(request);

    if (!result.success)
    {
        nb::Error::ErrorManager::instance().report(nb::Error::Type::FATAL, "Import failed: " + std::string(result.error.begin(), result.error.end()));
        return;
    }

    // лог результата (очень полезно для отладки Sponza)
    for (const auto& asset : result.assets)
    {
        std::string typeStr;

        switch (asset.type)
        {
        case nb::SDK::AssetType::MODEL:
            typeStr = "MODEL";
            break;
        case nb::SDK::AssetType::TEXTURE:
            typeStr = "TEXTURE";
            break;
        case nb::SDK::AssetType::MATERIAL:
            typeStr = "MATERIAL";
            break;
        }

        nb::Error::ErrorManager::instance().report(
            nb::Error::Type::FATAL, typeStr + " imported: " + asset.path.string()
        );
    }

    close();
}

nbui::LayoutBuilder ImportWindow::createOptionToggle(
    const std::wstring&   name,
    bool&                 value,
    std::function<void()> onClick
)
{
    using namespace nbui;
    return LayoutBuilder::hBox()
        .absoluteHeight(30)
        .margin({0, 1, 0, 0})
        .background({38, 38, 38})
        .child(
            LayoutBuilder::label(name)
                //.weight(1.0f)
                .fontSize(10)
                .margin({10, 7, 0, 0})
                .color({180, 180, 180})
        )
        .child(
            LayoutBuilder::widget(new Widgets::Button())
                .text(value ? L"ON" : L"OFF") // Тут лучше использовать Checkbox, если есть
                .absoluteWidth(60)
                .absoluteHeight(22)
                .margin({0, 4, 10, 0})
                .background({50, 50, 50})
                .apply<Widgets::Button>(
                    [this, onClick](Widgets::Button* b)
                    {
                        subscribe(b, &Widgets::Button::onPressedSignal, onClick);
                    }
                )
        );
}


std::unique_ptr<NNsLayout::LayoutNode> ImportWindow::buildUI()
{
    using namespace nbui;

    Widgets::TextEdit* assetNameEdit    = nullptr;
    Widgets::TextEdit* targetFolderEdit = nullptr;

    Widgets::CheckBox* cbImportMeshes     = nullptr;
    Widgets::CheckBox* cbImportMaterials  = nullptr;
    Widgets::CheckBox* cbImportTextures   = nullptr;
    Widgets::CheckBox* cbGenerateTangents = nullptr;
    Widgets::CheckBox* cbFlipUV           = nullptr;
    Widgets::CheckBox* cbCopyTextures     = nullptr;

    Widgets::ComboBox* compressionCombo = nullptr;
    Widgets::ComboBox* normalCombo      = nullptr;

    auto root =
        LayoutBuilder::vBox()
            .relativeHeight(1.0f)
            .relativeWidth(1.0f)
            .padding({10, 10, 10, 10})
            .background({28, 28, 28})

            // --- Source Path ---
            .child(
                LayoutBuilder::label(L"Source file")
                    .absoluteHeight(18.0f)
                    .relativeWidth(1.0f)
                    .fontSize(10)
                    .color({170, 170, 170})
            )
            .child(
                LayoutBuilder::label(settings.sourcePath.wstring())
                    .absoluteHeight(28.0f)
                    .relativeWidth(1.0f)
                    .background({35, 35, 35})
                    .color({220, 220, 220})
                    .margin({0, 2, 0, 10})
                    .fontSize(9)
            )

            // --- Asset Name ---
            .child(
                LayoutBuilder::label(L"Asset name")
                    .absoluteHeight(18.0f)
                    .relativeWidth(1.0f)
                    .fontSize(10)
                    .color({170, 170, 170})
            )
            .child(
                LayoutBuilder::widget(new Widgets::TextEdit())
                    .absoluteHeight(32.0f)
                    .relativeWidth(1.0f)
                    .margin({0, 2, 0, 10})
                    .apply<Widgets::TextEdit>(
                        [&](Widgets::TextEdit* e)
                        {
                            assetNameEdit = e;
                            e->setData(settings.assetName);

                            subscribe(
                                e, &Widgets::TextEdit::onTextChanged,
                                [this, e]()
                                {
                                    settings.assetName = e->getData();
                                }
                            );
                        }
                    )
            )

            // --- Target Folder ---
            .child(
                LayoutBuilder::label(L"Target folder")
                    .absoluteHeight(18.0f)
                    .relativeWidth(1.0f)
                    .fontSize(10)
                    .color({170, 170, 170})
            )
            .child(
                LayoutBuilder::widget(new Widgets::TextEdit())
                    .absoluteHeight(32.0f)
                    .relativeWidth(1.0f)
                    .margin({0, 2, 0, 10})
                    .apply<Widgets::TextEdit>(
                        [&](Widgets::TextEdit* e)
                        {
                            targetFolderEdit = e;
                            e->setData(settings.sourcePath.wstring());

                            subscribe(
                                e, &Widgets::TextEdit::onTextChanged,
                                [this, e]()
                                {
                                    settings.sourcePath = e->getData();
                                }
                            );
                        }
                    )
            )

            // --- Import Options ---
            .child(
                LayoutBuilder::label(L"Import options")
                    .absoluteHeight(18.0f)
                    .relativeWidth(1.0f)
                    .fontSize(10)
                    .color({170, 170, 170})
            )
            .child(
                LayoutBuilder::grid(2)
                    .absoluteHeight(140.0f)
                    .relativeWidth(1.0f)
                    .spacing(6.0f)
                    .margin({0, 5, 0, 10})

                    .child(
                        LayoutBuilder::widget(new Widgets::CheckBox())
                            .absoluteHeight(28.0f)
                            .relativeWidth(0.5f)
                            .text(L"Import meshes")
                            .apply<Widgets::CheckBox>(
                                [&](Widgets::CheckBox* cb)
                                {
                                    cbImportMeshes = cb;
                                    cb->setChecked(settings.importMeshes);

                                    subscribe(
                                        cb, &Widgets::CheckBox::onCheckStateChanged,
                                        [this, cb](bool flag)
                                        {
                                            settings.importMeshes = flag;
                                        }
                                    );
                                }
                            )
                    )

                    .child(
                        LayoutBuilder::widget(new Widgets::CheckBox())
                            .absoluteHeight(28.0f)
                            .relativeWidth(0.5f)
                            .text(L"Import materials")
                            .apply<Widgets::CheckBox>(
                                [&](Widgets::CheckBox* cb)
                                {
                                    cbImportMaterials = cb;
                                    cb->setChecked(settings.importMaterials);

                                    subscribe(
                                        cb, &Widgets::CheckBox::onCheckStateChanged,
                                        [this, cb](bool flag)
                                        {
                                            settings.importMaterials = flag;
                                        }
                                    );
                                }
                            )
                    )

                    .child(
                        LayoutBuilder::widget(new Widgets::CheckBox())
                            .absoluteHeight(28.0f)
                            .relativeWidth(0.5f)
                            .text(L"Import textures")
                            .apply<Widgets::CheckBox>(
                                [&](Widgets::CheckBox* cb)
                                {
                                    cbImportTextures = cb;
                                    cb->setChecked(settings.importTextures);

                                    subscribe(
                                        cb, &Widgets::CheckBox::onCheckStateChanged,
                                        [this, cb](bool flag)
                                        {
                                            settings.importTextures = flag;
                                        }
                                    );
                                }
                            )
                    )

                    .child(
                        LayoutBuilder::widget(new Widgets::CheckBox())
                            .absoluteHeight(28.0f)
                            .relativeWidth(0.5f)
                            .text(L"Generate tangents")
                            .apply<Widgets::CheckBox>(
                                [&](Widgets::CheckBox* cb)
                                {
                                    cbGenerateTangents = cb;
                                    cb->setChecked(settings.generateTangents);

                                    subscribe(
                                        cb, &Widgets::CheckBox::onCheckStateChanged,
                                        [this, cb](bool flag)
                                        {
                                            settings.generateTangents = flag;
                                        }
                                    );
                                }
                            )
                    )

                    .child(
                        LayoutBuilder::widget(new Widgets::CheckBox())
                            .absoluteHeight(28.0f)
                            .relativeWidth(0.5f)
                            .text(L"Flip UV")
                            .apply<Widgets::CheckBox>(
                                [&](Widgets::CheckBox* cb)
                                {
                                    cbFlipUV = cb;
                                    cb->setChecked(settings.flipUV);

                                    subscribe(
                                        cb, &Widgets::CheckBox::onCheckStateChanged,
                                        [this, cb](bool flag)
                                        {
                                            settings.flipUV = flag;
                                        }
                                    );
                                }
                            )
                    )

                    .child(
                        LayoutBuilder::widget(new Widgets::CheckBox())
                            .absoluteHeight(28.0f)
                            .relativeWidth(0.5f)
                            .text(L"Copy textures into project")
                            .apply<Widgets::CheckBox>(
                                [&](Widgets::CheckBox* cb)
                                {
                                    cbCopyTextures = cb;
                                    cb->setChecked(settings.copyTexturesIntoProject);

                                    subscribe(
                                        cb, &Widgets::CheckBox::onCheckStateChanged,
                                        [this, cb](bool flag)
                                        {
                                            settings.copyTexturesIntoProject = flag;
                                        }
                                    );
                                }
                            )
                    )
            )

            // --- Texture Settings ---
            .child(
                LayoutBuilder::label(L"Texture settings")
                    .absoluteHeight(18.0f)
                    .relativeWidth(1.0f)
                    .fontSize(10)
                    .color({170, 170, 170})
            )
            .child(
                LayoutBuilder::hBox()
                    .absoluteHeight(18.0f)
                    .relativeWidth(1.0f)
                    .margin({0, 5, 0, 4})
                    .child(
                        LayoutBuilder::label(L"Compression")
                            .relativeWidth(0.5f)
                            .absoluteHeight(18.0f)
                            .fontSize(9)
                            .color({140, 140, 140})
                    )
                    .child(
                        LayoutBuilder::label(L"Normal Map Convention")
                            .relativeWidth(0.5f)
                            .absoluteHeight(18.0f)
                            .fontSize(9)
                            .color({140, 140, 140})
                    )
            )
            .child(
                LayoutBuilder::hBox()
                    .absoluteHeight(34.0f)
                    .relativeWidth(1.0f)
                    .margin({0, 0, 0, 10})
                    .child(
                        LayoutBuilder::widget(new Widgets::ComboBox())
                            .relativeWidth(0.5f)
                            .absoluteHeight(34.0f)
                            .apply<Widgets::ComboBox>(
                                [&](Widgets::ComboBox* c)
                                {
                                    compressionCombo = c;
                                    c->addItem({L"None", 0});
                                    c->addItem({L"BC3", 1});
                                    c->addItem({L"BC7", 2});
                                    c->setSelectedItem(2);

                                    subscribe(
                                        c, &Widgets::ComboBox::onItemChecked,
                                        [this, c](const Widgets::ListItem& item)
                                        {
                                            switch (item.getValue<int>())
                                            {
                                            case 0:
                                                settings.textureCompression =
                                                    ImportSettings::TextureCompression::None;
                                                break;
                                            case 1:
                                                settings.textureCompression =
                                                    ImportSettings::TextureCompression::BC3;
                                                break;
                                            case 2:
                                                settings.textureCompression =
                                                    ImportSettings::TextureCompression::BC7;
                                                break;
                                            default:
                                                break;
                                            }
                                        }
                                    );
                                }
                            )
                    )
                    .child(
                        LayoutBuilder::widget(new Widgets::ComboBox())
                            .relativeWidth(0.5f)
                            .absoluteHeight(34.0f)
                            .apply<Widgets::ComboBox>(
                                [&](Widgets::ComboBox* c)
                                {
                                    normalCombo = c;
                                    c->addItem({L"OpenGL", 0});
                                    c->addItem({L"DirectX", 1});
                                    c->setSelectedItem(0);

                                    subscribe(
                                        c, &Widgets::ComboBox::onItemChecked,
                                        [this, c](const Widgets::ListItem& item)
                                        {
                                            settings.normalConvention =
                                                (item.getValue<int>() == 0)
                                                    ? ImportSettings::NormalConvention::OpenGL
                                                    : ImportSettings::NormalConvention::DirectX;
                                        }
                                    );
                                }
                            )
                    )
            )

            // --- Bottom buttons ---
            .child(
                LayoutBuilder::hBox()
                    .absoluteHeight(40.0f)
                    .relativeWidth(1.0f)
                    .margin({0, 10, 0, 0})
                    .child(
                        LayoutBuilder::widget(new Widgets::Button())
                            .text(L"Cancel")
                            .relativeWidth(0.5f)
                            .absoluteHeight(34.0f)
                            .background({50, 50, 50})
                            .apply<Widgets::Button>(
                                [&](Widgets::Button* b)
                                {
                                    subscribe(
                                        b, &Widgets::Button::onPressedSignal,
                                        [this]()
                                        {
                                            close();
                                        }
                                    );
                                }
                            )
                    )
                    .child(
                        LayoutBuilder::widget(new Widgets::Button())
                            .text(L"Import")
                            .relativeWidth(0.5f)
                            .absoluteHeight(34.0f)
                            .margin({8, 0, 0, 0})
                            .background({70, 120, 70})
                            .apply<Widgets::Button>(
                                [&](Widgets::Button* b)
                                {
                                    subscribe(
                                        b, &Widgets::IWidget::onReleasedSignal,
                                        [this]()
                                        {
                                            importAsset();
                                        }
                                    );
                                }
                            )
                    )
            );

    return std::move(root).build();
}
