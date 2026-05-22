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
    std::filesystem::path                     sourcePath,
    const std::function<void()>& onDestroyCallback
)
    : window()
    , engine(engine)
    , onDestroyCallback(onDestroyCallback)
{
    window = std::make_shared<Win32Window::ChildWindow>(nullptr);
    window->addCaption();

    importer = std::make_unique<nb::SDK::AssetImporter>();

    settings.sourcePath = sourcePath;
    updateAssetNameFromPath();

    window->getLayoutRoot()->addChild(buildUI());
    window->show();
}

ImportWindow::~ImportWindow() noexcept
{
    nb::Error::ErrorManager::instance().report(nb::Error::Type::INFO, "Import window destroyed");
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

    if (onDestroyCallback)
    {
        onDestroyCallback();
    }
}

void ImportWindow::importAsset()
{
    if (!engine || !importer)
    {
        return;
    }

    nb::SDK::ImportRequest request;

    request.sourceFile   = settings.sourcePath;
    request.targetFolder = settings.targetFolder;
    request.assetName    = settings.assetName;

    request.settings.importMeshes            = settings.importMeshes;
    request.settings.importMaterials         = settings.importMaterials;
    request.settings.importTextures          = settings.importTextures;
    request.settings.generateTangents        = settings.generateTangents;
    request.settings.flipUV                  = settings.flipUV;
    request.settings.copyTexturesIntoProject = settings.copyTexturesIntoProject;

    //request.settings.normalConvention =
    //    (settings.normalConvention == ImportSettings::NormalConvention::OpenGL)
    //        ? nb::SDK::ImportSettings::NormalConvention::OpenGL
    //        : nb::SDK::ImportSettings::NormalConvention::DirectX;

    auto result = importer->import(request);

    if (!result.success)
    {
        nb::Error::ErrorManager::instance().report(
            nb::Error::Type::FATAL, "Import failed: "// + wstring_to_string(result.error)
        );
        return;
    }

    for (const auto& asset : result.assets)
    {
        nb::Error::ErrorManager::instance().report(
            nb::Error::Type::FATAL, "Imported: " + asset.path.string()
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
                .text(value ? L"ON" : L"OFF") 
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

    auto propertyRow = [](const std::wstring& label, LayoutBuilder&& widgetBuilder)
    {
        return LayoutBuilder::hBox()
            .absoluteHeight(28)
            .margin({4, 2, 4, 2})
            .child(
                LayoutBuilder::label(label).relativeWidth(0.45f).fontSize(9).color({160, 160, 160})
            )
            .child(std::move(widgetBuilder).relativeWidth(0.55f));
    };

    auto root =
        LayoutBuilder::vBox()
            .relativeHeight(1.0f)
            .relativeWidth(1.0f)
            .background({30, 30, 30})

            .child(
                LayoutBuilder::vBox()
                    .autoHeight()
                    .padding({12, 8, 12, 8})
                    .background({22, 22, 22})
                    .child(
                        LayoutBuilder::label(L"IMPORTING ASSET").fontSize(8).color({100, 100, 100})
                    )
                    .child(
                        LayoutBuilder::label(settings.sourcePath.filename().wstring())
                            .fontSize(11)
                            .color({0, 160, 255})
                    )
            )

            .child(
                LayoutBuilder::vBox()
                    .autoHeight()
                    .padding({8, 8, 8, 8})

                    .child(
                        LayoutBuilder::section(L"COMMON", false)
                            .child(propertyRow(
                                L"Asset Name", LayoutBuilder::widget(new Widgets::TextEdit())
                                                   .apply<Widgets::TextEdit>(
                                                       [&](auto* e)
                                                       {
                                                           e->setData(settings.assetName);
                                                           subscribe(
                                                               e, &Widgets::TextEdit::onTextChanged,
                                                               [this, e]
                                                               {
                                                                   settings.assetName =
                                                                       e->getData();
                                                               }
                                                           );
                                                       }
                                                   )
                            ))
                            .child(propertyRow(
                                L"Target Folder",
                                LayoutBuilder::widget(new Widgets::TextEdit())
                                    .apply<Widgets::TextEdit>(
                                        [&](auto* e)
                                        {
                                            e->setData(settings.targetFolder.wstring());
                                            subscribe(
                                                e, &Widgets::TextEdit::onTextChanged,
                                                [this, e]
                                                {
                                                    settings.targetFolder = e->getData();
                                                }
                                            );
                                        }
                                    )
                            ))
                    )

                    .child(
                        LayoutBuilder::section(L"GEOMETRY", false)
                            .child(propertyRow(
                                L"Import Meshes",
                                LayoutBuilder::widget(new Widgets::CheckBox())
                                    .checked(settings.importMeshes)
                                    .apply<Widgets::CheckBox>(
                                        [&](auto* cb)
                                        {
                                            subscribe(
                                                cb, &Widgets::CheckBox::onCheckStateChanged,
                                                [this](bool f)
                                                {
                                                    settings.importMeshes = f;
                                                }
                                            );
                                        }
                                    )
                            ))
                            .child(propertyRow(
                                L"Generate Tangents",
                                LayoutBuilder::widget(new Widgets::CheckBox())
                                    .checked(settings.generateTangents)
                                    .apply<Widgets::CheckBox>(
                                        [&](auto* cb)
                                        {
                                            subscribe(
                                                cb, &Widgets::CheckBox::onCheckStateChanged,
                                                [this](bool f)
                                                {
                                                    settings.generateTangents = f;
                                                }
                                            );
                                        }
                                    )
                            ))
                    )

                    .child(
                        LayoutBuilder::section(L"MATERIALS", false)
                            .child(propertyRow(
                                L"Import Materials",
                                LayoutBuilder::widget(new Widgets::CheckBox())
                                    .checked(settings.importMaterials)
                                    .apply<Widgets::CheckBox>(
                                        [&](auto* cb)
                                        {
                                            subscribe(
                                                cb, &Widgets::CheckBox::onCheckStateChanged,
                                                [this](bool f)
                                                {
                                                    settings.importMaterials = f;
                                                }
                                            );
                                        }
                                    )
                            ))
                            .child(propertyRow(
                                L"Flip UVs",
                                LayoutBuilder::widget(new Widgets::CheckBox())
                                    .checked(settings.flipUV)
                                    .apply<Widgets::CheckBox>(
                                        [&](auto* cb)
                                        {
                                            subscribe(
                                                cb, &Widgets::CheckBox::onCheckStateChanged,
                                                [this](bool f)
                                                {
                                                    settings.flipUV = f;
                                                }
                                            );
                                        }
                                    )
                            ))
                    )

                    .child(
                        LayoutBuilder::section(L"ADVANCED TEXTURES", true)
                            .child(propertyRow(
                                L"Compression", LayoutBuilder::widget(new Widgets::ComboBox())
                                                    .apply<Widgets::ComboBox>(
                                                        [&](auto* c)
                                                        {
                                                            c->addItem({L"None", 0});
                                                            c->addItem({L"BC3", 1});
                                                            c->addItem({L"BC7", 2});
                                                            c->setSelectedItem(2);
                                                        }
                                                    )
                            ))
                    )
            )
            .child(LayoutBuilder::spacer())

            .child(
                LayoutBuilder::hBox()
                    .absoluteHeight(30)
                    //.padding({12, 10, 12, 10})
                    .background({22, 22, 22})
                    //.child(LayoutBuilder::spacer()) // Пружина: прижимает кнопки вправо
                    .child(
                        LayoutBuilder::widget(new Widgets::Button())
                            .text(L"Cancel")
                            .absoluteHeight(30)
                            .absoluteWidth(90)
                            .background({55, 55, 55})
                            .apply<Widgets::Button>(
                                [this](auto* b)
                                {
                                    subscribe(
                                        b, &Widgets::Button::onPressedSignal,
                                        [this]
                                        {
                                            close();
                                        }
                                    );
                                }
                            )
                    )
                    .child(
                        LayoutBuilder::widget(new Widgets::Button())
                            .text(L"IMPORT")
                            .absoluteHeight(30)
                            .absoluteWidth(110)
                            .margin({0, 0, 0, 0})
                            .background({0, 110, 190}) 
                            .onEvent(&Widgets::Button::onReleasedSignal, [this]
                             {
                                 importAsset();
                             })
                            
                    )
            );

    return std::move(root).build();
}