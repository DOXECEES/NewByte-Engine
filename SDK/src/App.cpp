// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "App.hpp"
#include "Core.hpp"
#include "TextureEditor.hpp"
#include "Widgets/Button.hpp"
#include "Win32Window/Win32ChildWindow.hpp"


// ENGINE HEADERS
#include <Error/ErrorConsolePrinter.hpp>
#include <Common/StringUtils.hpp>


// NBUI HEADERS
#include <Localization/LocaleManager.hpp>
#include <Localization/LocaleLoader.hpp>
#include <Localization/Translation.hpp>
#include <Localization/Calendars/HebrewCalendar.hpp>
#include <Signal.hpp>

#include <LayoutBuilder.hpp>

#include <Widgets/TreeView.hpp>
#include <Widgets/SpinBox.hpp>
#include <Widgets/Calendar.hpp>
#include <Widgets/ComboBox.hpp>
#include <Widgets/Slider.hpp>

#include <Widgets/Section.hpp>

#include <Widgets/ColorPicker.hpp>
#include <Widgets/ToolBar.hpp>
#include <Widgets/MaterialWidget.hpp>
#include <Widgets/FilePicker.hpp>

#include <Renderer/Shader.hpp>
#include <Renderer/Material.hpp>
#include <Renderer/Texture.hpp>
#include <Renderer/Mesh.hpp>

#include <Renderer/Objects/Objects.hpp>
#include <Renderer/Scene.hpp>
#include <memory>

#include <Physics/Physics.hpp>
#include <Serialize/JsonArchive.hpp>

#include "ComponentBrowser.hpp"
#include "PrimitiveCreationDialog.hpp"


void EditorApp::openColorPickerWindow()
{
    colorPickerWindow = std::make_shared<Win32Window::ModalWindow>(NbSize<int>{300,300}, inspectorWindow.get());
    colorPickerWindow->setTitle(L"Color picker");

    using namespace nbui;
    auto ui =
        LayoutBuilder::vBox()
            .style(
                [this](auto& s)
                {
                    // s.padding = {10, 10, 10, 10};
                    s.color = {30, 30, 30};
                }
            )
            .child(
                LayoutBuilder::widget(new Widgets::ColorPicker({}))
                    .relativeHeight(1.0f)
                    .relativeWidth(1.0f)
                    .onEvent(
                        &Widgets::ColorPicker::onCancelButtonPressed,
                        [this]()
                        {
                            SendMessage(colorPickerWindow->getHandle().as<HWND>(), WM_CLOSE, 0, 0);
                        }
                    )
                    .onEvent(
                        &Widgets::ColorPicker::onOkButtonPressed,
                        [this](const nb::Color& color)
                        {
                            nb::Error::ErrorManager::instance()
                                .report(nb::Error::Type::INFO, "Color")
                                .with("X", color.asVec4().x)
                                .with("Y", color.asVec4().y)
                                .with("Z", color.asVec4().z)
                                .with("A", color.asVec4().w);
                        }
                    )

            )
            .build();
    colorPickerWindow->getLayoutRoot()->addChild(std::move(ui));
}

void EditorApp::openFilePickerWindow()
{
    filePickerWindow =
        std::make_shared<Win32Window::ModalWindow>(NbSize<int>{300, 300}, inspectorWindow.get());
    filePickerWindow->setTitle(L"Color picker");

    using namespace nbui;
    auto ui =
        LayoutBuilder::vBox()
            .style(
                [this](auto& s)
                {
                    // s.padding = {10, 10, 10, 10};
                    s.color = {30, 30, 30};
                }
            )
            .child(
                LayoutBuilder::widget(new Widgets::FilePicker({}))
                    .relativeHeight(1.0f)
                    .relativeWidth(1.0f)
                    
                    

            )
            .build();
    filePickerWindow->getLayoutRoot()->addChild(std::move(ui));
}

void EditorApp::initSystems() noexcept
{
    ::_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    ::AllocConsole();
    
    nb::Error::ErrorManager::instance()
        .setPrinter(new nb::Error::ErrorConsolePrinter());
    
    setAppLocale();
}

void EditorApp::setAppLocale() noexcept
{
    using namespace Localization;
    Locale locale = LocalLoader::load("/Assets/Locales/ru-RU.locale");
    LocaleManager::setCurrent(locale);
    Translation::load("Assets/Localization/ru.translation");
}

void EditorApp::createWindows() noexcept
{
    using namespace Localization;

    mainWindow = std::make_shared<Win32Window::Window>();
    mainWindow->setTitle(
        Utils::toWstring(Translation::fromKey("Ui.Editor.Title"))
    );
    //mainWindow->excludeFromClientRect({32, 0, 0, 0});


    sceneWindow = std::make_shared<Win32Window::ChildWindow>(mainWindow.get(), true);
    sceneWindow->setTitle(
        Utils::toWstring(Translation::fromKey("Ui.Editor.Scene.Title"))
    );

    hierarchyWindow = std::make_shared<Win32Window::ChildWindow>(mainWindow.get());
    hierarchyWindow->setTitle(
        Utils::toWstring(Translation::fromKey("Ui.Editor.Hierarchy.Title"))
    );

    inspectorWindow = std::make_shared<Win32Window::ChildWindow>(mainWindow.get());
    inspectorWindow->setTitle(
        Utils::toWstring(Translation::fromKey("Ui.Editor.Inspector.Title"))
    );

    assetManager = std::make_shared<Win32Window::ChildWindow>(mainWindow.get());
    assetManager->setTitle(L"Asset");
    // textureInspector = std::make_shared<Win32Window::ChildWindow>(mainWindow.get(), true);
    // textureInspector->setTitle(
    //     Utils::toWstring(Translation::fromKey("Ui.Editor.TextureView.Title"))
    // );

    debugWindow = std::make_shared<Win32Window::ChildWindow>(mainWindow.get());
    debugWindow->setTitle(
        Utils::toWstring(Translation::fromKey("Ui.Editor.DebugWindow.Title"))
    );

    toolbarWindow = std::make_shared<Win32Window::ChildWindow>(mainWindow.get());
    toolbarWindow->setTitle(L"Toolbar");

    //tempWindow = std::make_shared<Win32Window::ChildWindow>(mainWindow.get());
    //tempWindow->setTitle(L"tempWindow");


}

void EditorApp::setupDocking() noexcept
{
    dockManager = std::make_unique<Temp::DockingSystem>(mainWindow);

    auto sceneTab = dockManager->dockAsTab(sceneWindow, nullptr, "Scene");

    dockManager->dockRelative(
        hierarchyWindow, Temp::DockPosition::LEFT, sceneWindow, Temp::Percent(20)
    );

    dockManager->dockRelative(
        inspectorWindow, Temp::DockPosition::RIGHT, nullptr, Temp::Percent(25)
    );

     dockManager->dockRelative(
        debugWindow, Temp::DockPosition::BOTTOM, inspectorWindow, Temp::Percent(50)
    );


    dockManager->dockRelative(
        assetManager, Temp::DockPosition::BOTTOM, nullptr, Temp::Percent(40)
    );

    dockManager->dockRelative(toolbarWindow, Temp::DockPosition::TOP, nullptr, Temp::Percent(3));

   
    subscribe(
        *mainWindow, &Win32Window::Window::onRectChanged,
        [this](const NbRect<int>& rect)
        {
            dockManager->onSize(rect.width, rect.height);
        }
    );
}

void EditorApp::initEngine() noexcept
{
    engine = std::make_shared<nb::Core::Engine>(sceneWindow->getHandle().as<HWND>());
    auto& scene = nb::Scene::getInstance();

    sceneModel = std::make_shared<SceneModelEcs>(scene.getRegistry(), scene.getRootEntity().id);
    
    const auto& size = sceneWindow->getSize();
    nb::Core::EngineSettings::setHeight(size.height);
    nb::Core::EngineSettings::setWidth(size.width);

    subscribe(*sceneWindow, &Win32Window::ChildWindow::onSizeChanged, [](const NbSize<int>& s) {
        nb::Core::EngineSettings::setHeight(s.height);
        nb::Core::EngineSettings::setWidth(s.width);
    });


}

void EditorApp::setupMainWindow() noexcept
{
    using namespace nbui;
    auto rootUI =
        LayoutBuilder::vBox()
            .style(
                [](NNsLayout::LayoutStyle& s)
                {
                    s.width          = 1.0f;
                    s.height         = 1.0f;
                    s.widthSizeType  = NNsLayout::SizeType::RELATIVE;
                    s.heightSizeType = NNsLayout::SizeType::RELATIVE;
                    s.color          = NbColor{30, 30, 30};
                }
            )
            .child(
                LayoutBuilder::toolbar()
                    .buttonGroupOnlyOne()
                    .relativeHeight(1.0f)
                    .absoluteWidth(120)
                    .child(
                        LayoutBuilder::widget(new Widgets::Button())
                            .text(L"Save")
                            .absoluteWidth(50)
                            .relativeHeight(1.0f)
                            .onEvent(
                                &Widgets::Button::onPressedSignal,
                                [this]()
                                {
                                    engine->saveSnapshot();
                                }
                            )
                    )
                    .child(
                        LayoutBuilder::widget(new Widgets::Button())
                            .text(L"Load")
                            .absoluteWidth(50)
                            .relativeHeight(1.0f)
                            .onEvent(
                                &Widgets::Button::onPressedSignal,
                                [this]()
                                {
                                    engine->loadSnapshot();
                                }
                            )
                    )
                    .endGroup()
            )
            .build();

    toolbarWindow->getLayoutRoot()->addChild(std::move(rootUI));
}

void EditorApp::setupHierarchyUI() noexcept
{
    using namespace nbui;
    //using namespace EditorTheme; // Предполагаем наличие общей темы

    auto ui = LayoutBuilder::vBox()
                  .style(
                      [](auto& s)
                      {
                          s.padding = {10, 10, 10, 10};
                          s.color   = {30, 30, 30};
                      }
                  )
                  .child(
                      LayoutBuilder::treeView()
                          .relativeWidth(1.0f)
                          .relativeHeight(1.0f)
                          .apply<Widgets::TreeView>(
                              [this](auto* tv)
                              {
                                  tv->setModel(sceneModel);
                                  this->setupHierarchyEvents(tv);
                              }
                          )
                  )
                  .build();

    hierarchyWindow->getLayoutRoot()->addChild(std::move(ui));
}

void EditorApp::setupInspectorUI() noexcept
{
    
}

nbui::LayoutBuilder EditorApp::createSpinBox(std::function<void(int)> onChange, const NbColor& color)
{
    return nbui::LayoutBuilder::vBox()
        .relativeWidth(0.33f)
        .child(nbui::LayoutBuilder::widget(new Widgets::FloatSpinBox())
            .apply<Widgets::FloatSpinBox>(
                [](auto* c)
                {
                    c->setRange(-100, 100);
                }
            )
            .background({ 25, 25, 25 })
            .color(color));
            //.onEvent(&Widgets::SpinBox::onValueChangedByStep, onChange));

}


auto createAssetCard(
    const std::wstring& name,
    const std::wstring& type,
    NbColor typeColor
)
{
    using namespace nbui;

    return LayoutBuilder::vBox()
        .style(
            [typeColor](NNsLayout::LayoutStyle& s)
            {
                s.widthSizeType = NNsLayout::SizeType::ABSOLUTE;
                s.width = 110;
                s.heightSizeType = NNsLayout::SizeType::ABSOLUTE;
                s.height = 145;
                s.margin = {5, 5, 5, 5};
                s.padding = {0, 0, 0, 0};
                s.color = NbColor{35, 35, 35};
            }
        )
        // 1. Превью (80px)
        .child(
            LayoutBuilder::label(L"").absoluteWidth(110).absoluteHeight(80).background(
                NbColor{20, 20, 20}
            )
        )

        // 2. Цветовой индикатор типа (5px)
        .child(LayoutBuilder::label(L"").absoluteWidth(110).absoluteHeight(5).background(typeColor))

        // 3. Имя (35px)
        .child(
            LayoutBuilder::label(name)
                .absoluteWidth(110)
                .absoluteHeight(35)
                .fontSize(12)
                .color(NbColor{220, 220, 220})
                //.textAlignment({})
        )

        // 4. Текст типа (25px)
        .child(
            LayoutBuilder::label(type)
                .absoluteWidth(110)
                .absoluteHeight(25)
                .fontSize(10)
                .color(NbColor{130, 130, 130})
                //.textAlign(Widgets::TextAlign::CENTER)
        );
}

void EditorApp::setupSettingsUI() noexcept
{
    using namespace nbui;
    //modal =
    //    std::make_shared<Win32Window::ModalWindow>(NbSize<int>{1280, 720}, debugWindow.get());
    //modal->setTitle(L"Content Browser");


    

    auto ui =
        LayoutBuilder::hBox()
            .style(
                [](NNsLayout::LayoutStyle& s)
                {
                    s.widthSizeType = NNsLayout::SizeType::RELATIVE;
                    s.width = 1.0f;
                    s.heightSizeType = NNsLayout::SizeType::RELATIVE;
                    s.height = 1.0f;
                    s.color = NbColor{25, 25, 25};
                    s.padding = {0, 0, 0, 0};
                }
            )
            // ==========================================
            // ЛЕВАЯ ПАНЕЛЬ: ДЕРЕВО (25% ширины)
            // ==========================================
            .child(
                LayoutBuilder::vBox()
                    .style(
                        [](NNsLayout::LayoutStyle& s)
                        {
                            s.widthSizeType = NNsLayout::SizeType::RELATIVE;
                            s.width = 0.25f;
                            s.heightSizeType = NNsLayout::SizeType::RELATIVE;
                            s.height = 1.0f;
                            s.color = NbColor{40, 40, 40};
                        }
                    )
                    .child(
                        LayoutBuilder::label(L"CONTENT TREE")
                            .relativeWidth(1.0f)
                            .absoluteHeight(50)
                            .background(NbColor{60, 60, 60})
                            .color(NbColor{220, 220, 220})
                            //.textAlign(Widgets::TextAlign::CENTER)
                    )

                    .child(
                        LayoutBuilder::widget(new Widgets::Button())
                            .text(L"📁 Assets")
                            .relativeWidth(1.0f)
                            .absoluteHeight(40)
                            .background(NbColor{50, 50, 50})
                    )
                    .child(
                        LayoutBuilder::widget(new Widgets::Button())
                            .text(L"    📁 Models")
                            .relativeWidth(1.0f)
                            .absoluteHeight(40)
                            .background(NbColor{45, 45, 45})
                    )
                    .child(
                        LayoutBuilder::widget(new Widgets::Button())
                            .text(L"    📁 Textures")
                            .relativeWidth(1.0f)
                            .absoluteHeight(40)
                            .background(NbColor{45, 45, 45})
                    )

                    .child(LayoutBuilder::spacer()) // Пружина: прижимает кнопки к верху
            )

            // ==========================================
            // ПРАВАЯ ПАНЕЛЬ: КОНТЕНТ (75% ширины)
            // ==========================================
            .child(
                LayoutBuilder::vBox()
                    .style(
                        [](NNsLayout::LayoutStyle& s)
                        {
                            s.widthSizeType = NNsLayout::SizeType::RELATIVE;
                            s.width = 0.75f;
                            s.heightSizeType = NNsLayout::SizeType::RELATIVE;
                            s.height = 1.0f;
                            s.padding = {10, 10, 10, 10};
                            s.color = NbColor{30, 30, 30};
                        }
                    )

                    // 1. Тулбар (Фиксированная высота 45px)
                    .child(
                        LayoutBuilder::hBox()
                            .style(
                                [](NNsLayout::LayoutStyle& s)
                                {
                                    s.widthSizeType = NNsLayout::SizeType::RELATIVE;
                                    s.width = 1.0f;
                                    s.heightSizeType = NNsLayout::SizeType::ABSOLUTE;
                                    s.height = 45;
                                    s.margin = {0, 0, 10, 0};
                                }
                            )
                            .child(
                                LayoutBuilder::label(L"SEARCH:")
                                    .absoluteWidth(80)
                                    .absoluteHeight(45)
                                    .color(NbColor{150, 150, 150})
                            )
                            .child(
                                LayoutBuilder::widget(new Widgets::Button())
                                    .text(L"Filter assets...")
                                    .relativeWidth(0.6f)
                                    .absoluteHeight(45)
                                    .background(NbColor{20, 20, 20})
                            )
                            .child(LayoutBuilder::spacer())
                            .child(
                                LayoutBuilder::widget(new Widgets::Button())
                                    .text(L"IMPORT")
                                    .absoluteWidth(100)
                                    .absoluteHeight(45)
                                    .background(NbColor{0, 120, 215})
                            )
                    )

                    // 2. Сетка ассетов (Занимает основную часть экрана)
                    .child(
                        LayoutBuilder::flow()
                            .style(
                                [](NNsLayout::LayoutStyle& s)
                                {
                                    s.widthSizeType = NNsLayout::SizeType::RELATIVE;
                                    s.width = 1.0f;
                                    s.heightSizeType = NNsLayout::SizeType::AUTO;
                                    //s.padding = {10, 10, 10, 10};
                                }
                            )
                           
                                    .child(createAssetCard(
                                        L"Wall_D", L"Texture2D", NbColor{76, 175, 80}
                                    ))
                                    .child(createAssetCard(
                                        L"Wall_N", L"Texture2D", NbColor{76, 175, 80}
                                    ))
                                    .child(createAssetCard(
                                        L"Brick_M", L"Material", NbColor{255, 152, 0}
                                    ))
                                    .child(createAssetCard(
                                        L"Rock_Mesh", L"StaticMesh", NbColor{33, 150, 243}
                                    ))
                                    .child(createAssetCard(
                                        L"PBR_Std", L"Shader", NbColor{156, 39, 176}
                                    ))
                                    .child(createAssetCard(
                                        L"PBR_Std", L"Shader", NbColor{156, 39, 176}
                                    ))
                                    .child(createAssetCard(
                                        L"PBR_Std", L"Shader", NbColor{156, 39, 176}
                                    ))
                                    .child(createAssetCard(
                                        L"PBR_Std", L"Shader", NbColor{156, 39, 176}
                                    ))
                                    .child(createAssetCard(
                                        L"PBR_Std", L"Shader", NbColor{156, 39, 176}
                                    ))
                                    .child(createAssetCard(
                                        L"PBR_Std", L"Shader", NbColor{156, 39, 176}
                                    ))
                                    .child(createAssetCard(
                                        L"PBR_Std", L"Shader", NbColor{156, 39, 176}
                                    ))
                                    .child(createAssetCard(
                                        L"PBR_Std", L"Shader", NbColor{156, 39, 176}
                                    ))
                                    .child(createAssetCard(
                                        L"PBR_Std", L"Shader", NbColor{156, 39, 176}
                                    ))
                                    .child(createAssetCard(
                                        L"PBR_Std", L"Shader", NbColor{156, 39, 176}
                                    ))
                                    .child(createAssetCard(
                                        L"PBR_Std", L"Shader", NbColor{156, 39, 176}
                                    ))
                                    .child(createAssetCard(
                                        L"PBR_Std", L"Shader", NbColor{156, 39, 176}
                                    ))
                                    .child(LayoutBuilder::spacer())
                            
                            
                            
                            .child(LayoutBuilder::spacer()) // Растягивает внутренности сетки вверх
                    )

                    // !!! МАГИЯ: ЭТОТ СПЕЙСЕР ПРИЖИМАЕТ ВСЁ, ЧТО НИЖЕ, К НИЗУ ПРАВОЙ ПАНЕЛИ !!!
                    .child(LayoutBuilder::spacer())

                    // 3. Статус-бар (Фиксированная высота 30px, всегда внизу)
                    .child(
                        LayoutBuilder::hBox()
                            .style(
                                [](NNsLayout::LayoutStyle& s)
                                {
                                    s.widthSizeType = NNsLayout::SizeType::RELATIVE;
                                    s.width = 1.0f;
                                    s.heightSizeType = NNsLayout::SizeType::ABSOLUTE;
                                    s.height = 30;
                                    s.color = NbColor{45, 45, 45};
                                }
                            )
                            .child(
                                LayoutBuilder::label(L"  Items: 5 | Memory: 14.2 MB | Filter: None")
                                    .relativeWidth(1.0f)
                                    .absoluteHeight(30)
                                    .fontSize(11)
                                    .margin({0,0,0,0})
                                    .color(NbColor{180, 180, 180})
                                    .textAlignment({.textAlignment = TextAlignment::LEFT})

                            )
                    )
            )
            .build();

    //modal->getLayoutRoot()->addChild(std::move(ui));
    //modal->show();
}

void EditorApp::setupEngineDependentUi() noexcept
{
    setupDebugUI();
    setupAssetManager();
    debugWindow->show();
}

void EditorApp::setupDebugUI() noexcept
{
    using namespace nbui;
    auto debugUI = LayoutBuilder::vBox()
        .style([](NNsLayout::LayoutStyle& s) {
        s.widthSizeType = NNsLayout::SizeType::ABSOLUTE;
        s.width = 250;
        s.heightSizeType = NNsLayout::SizeType::RELATIVE;
        s.height = 1.0f;
        s.color = NbColor{ 35, 35, 35 };
        s.padding = { 10, 10, 10, 10 };
            })

        .child(LayoutBuilder::label(L"VISUALIZATION")
            .relativeWidth(1.0f).absoluteHeight(25)
            .color(NbColor{ 150, 150, 150 }).fontSize(12))

        .child(LayoutBuilder::widget(new Widgets::CheckBox())
            .text(L"Wireframe Mode")
            .relativeWidth(1.0f).absoluteHeight(30)
            .onEvent(&Widgets::CheckBox::onCheckStateChanged, [&](bool checked) {
                engine->getRenderer()->setWireframeMode(checked);
            }))

        .child(LayoutBuilder::widget(new Widgets::CheckBox())
            .text(L"Show grid")
            .relativeWidth(1.0f).absoluteHeight(30)
            .apply<Widgets::CheckBox>(
                [](Widgets::CheckBox* checkbox)
                {
                    checkbox->setChecked(true);
                }
            )
            .onEvent(&Widgets::CheckBox::onCheckStateChanged, [&](bool checked) {
                engine->getRenderer()->toggleGridShow();
            }))

        .child(LayoutBuilder::widget(new Widgets::CheckBox())
            .text(L"Show light sources")
            .relativeWidth(1.0f).absoluteHeight(30)
            .onEvent(&Widgets::CheckBox::onCheckStateChanged, [&](bool checked) {
                engine->getRenderer()->toggleDebugPass();
                }))
            .child(
                LayoutBuilder::widget(new Widgets::CheckBox())
                    .text(L"Show BVH bounds")
                    .relativeWidth(1.0f)
                    .absoluteHeight(30)
                    .onEvent(
                        &Widgets::CheckBox::onCheckStateChanged,
                        [&](bool checked)
                        {
                            engine->getRenderer()->toggleBvhVisualization();
                        }
                    )
            )

        // Разделитель
        .child(LayoutBuilder::spacer().absoluteHeight(10))

        // --- СЕКЦИЯ: ИСТОЧНИКИ СВЕТА ---
        .child(LayoutBuilder::label(L"LIGHTING & GIZMOS")
            .relativeWidth(1.0f).absoluteHeight(25)
            .color(NbColor{ 150, 150, 150 }).fontSize(12))

        .child(LayoutBuilder::widget(new Widgets::CheckBox())
            .text(L"Show Light Icons")
            .relativeWidth(1.0f).absoluteHeight(30)
            .onEvent(&Widgets::CheckBox::onCheckStateChanged, [](bool checked) {
                //g_engine->getRenderer()->setIconsVisible(Renderer::IconType::Light, checked);
                }))

        .child(LayoutBuilder::widget(new Widgets::CheckBox())
            .text(L"Show Bounding Boxes")
            .relativeWidth(1.0f).absoluteHeight(30)
            .onEvent(&Widgets::CheckBox::onCheckStateChanged, [&](bool checked) {
                            engine->getRenderer()->toggleBoundingBoxVisualization();
            }))

        .child(LayoutBuilder::widget(new Widgets::CheckBox())
            .text(L"Enable Shadows")
            .relativeWidth(1.0f).absoluteHeight(30)
            .onEvent(&Widgets::CheckBox::onCheckStateChanged, [](bool checked) {
                //g_engine->getRenderer()->setShadowsEnabled(checked);
                }))

        // Разделитель
        .child(LayoutBuilder::spacer().absoluteHeight(10))

        // --- СЕКЦИЯ: СТАТИСТИКА ---
        .child(LayoutBuilder::label(L"STATISTICS")
            .relativeWidth(1.0f).absoluteHeight(25)
            .color(NbColor{ 150, 150, 150 }).fontSize(12))


        .child(LayoutBuilder::widget(new Widgets::ComboBox())
            .apply<Widgets::ComboBox>([&](Widgets::ComboBox* c) {

                const nb::Renderer::Renderer* renderer = engine->getRenderer().get();

                c->addItem({ L"Albedo",     renderer->getAlbedoId()});
                c->addItem({ L"Ao",         renderer->getAoId() });
                c->addItem({ L"Metal",      renderer->getMetalId() });
                c->addItem({ L"Normal",     renderer->getNormalId()});
                c->addItem({ L"Roughtness", renderer->getRoughtnessId()});
                c->addItem({ L"Shadow",     renderer->getShadowTextureId() });
                c->addItem({ L"Gizmo",      renderer->getGizmoTextureId() });

             })
            .text(L"Show Draw Calls")
            .relativeWidth(1.0f).absoluteHeight(30)
            .onEvent(&Widgets::ComboBox::onItemChecked, [&](const Widgets::ListItem& item) {
                
                engine->getRenderer()->setCheckedTextureId(item.getValue<uint32_t>());
            }))
        .child(LayoutBuilder::widget(new Widgets::CheckBox())
            .text(L"Show FPS Counter")
            .relativeWidth(1.0f).absoluteHeight(30)
            .onEvent(&Widgets::CheckBox::onCheckStateChanged, [](bool checked) {
                //g_engine->getUI()->setOverlayVisible(L"FPS", checked);
                }))

        .child(LayoutBuilder::spacer()) // Пружина, чтобы все прижалось к верху
        .build();

    debugWindow->getLayoutRoot()->addChild(std::move(debugUI));
}

void EditorApp::setupAssetManager() noexcept
{
    using namespace nbui;
    
    auto res = nb::ResMan::ResourceManager::getInstance();
    
}

void EditorApp::rebuildInspector() noexcept
{
    using namespace nbui;

    nbui::GlobalWidgetContext::releasePressedWidget();


    if (activeNode.getId() == 0)
    {
        inspectorWindow->getLayoutRoot()->clearChilds();
        return;
    }

    auto inspectorBuilder = LayoutBuilder::vBox().style(
        [](NNsLayout::LayoutStyle& s)
        {
            s.widthSizeType = NNsLayout::SizeType::RELATIVE;
            s.width = 1.0f;
            s.heightSizeType = NNsLayout::SizeType::RELATIVE;
            s.height         = 1.0f;
            s.color = NbColor{35, 35, 35};
        }
    );

    auto& registry = nb::Scene::getInstance().getRegistry();
    auto entityId = activeNode.getId();

    for (auto& storage : registry.getAllStorages())
    {
        if (storage && storage->contains(entityId))
        {
            auto info = storage->getTypeInfo();

            if (info->isInternal)
            {
                continue;
            }

            auto data = storage->getRaw(entityId);

            inspectorBuilder = std::move(inspectorBuilder)
                                   .child(
                                       LayoutBuilder::label(nb::Utils::toWString(info->name))
                                           .relativeWidth(1.0f)
                                           .absoluteHeight(30)
                                           .background({60, 60, 60})
                                           .color({220, 220, 220})
                                           .textAlignment({.textAlignment = TextAlignment::LEFT})
                                   );

            for (const auto& field : info->fields)
            {
                inspectorBuilder = buildFieldUI(std::move(inspectorBuilder), data, info, field);
            }
            inspectorBuilder =
                std::move(inspectorBuilder).child(LayoutBuilder::spacerAbsolute(1.0f, 5.0f));

        }
    }

    if (activeNode.isValid())
    {
        inspectorBuilder = std::move(inspectorBuilder)
            .child(
                LayoutBuilder::widget(new Widgets::Button)
                    .relativeWidth(1.0f)
                    .absoluteHeight(40.0f)
                    .onEvent(
                        &Widgets::Button::onReleasedSignal,
                        [&]()
                        {
                            auto& browser = nbui::ComponentBrowser::get();

                            browser.clear();

                            auto& registry = nb::Scene::getInstance().getRegistry();
                            auto  entityId = activeNode.getId();

                            for (auto& storage : registry.getAllStorages())
                            {
                                if (!storage)
                                {
                                    continue;
                                }

                                auto*       typeInfo = storage->getTypeInfo();
                                std::string compName = (typeInfo->name);

                                if (!storage->contains(entityId))
                                {
                                    auto rawStorage = storage.get();
                                    browser.addItem(
                                        Utils::toWstring(compName),
                                        [this, rawStorage, entityId]()
                                        {
                                            rawStorage->addDefault(entityId);
                                            this->rebuildInspector();
                                            nb::Scene::getInstance().invalidateBvh();
                                        }
                                    );
                                }
                            }

                            auto pos = this->mainWindow->getMousePosition();

                            browser.show(this->mainWindow->getHandle().as<HWND>(), pos.x, pos.y);
                        }
                    )
                    .text(L"Добавить компонент")
            );
    }



    auto finalUi = std::move(inspectorBuilder).build();

    inspectorWindow->getLayoutRoot()->clearChilds();
    inspectorWindow->getLayoutRoot()->addChild(std::move(finalUi));
    inspectorWindow->show();
}

void EditorApp::subscribeAll() noexcept
{
    subscribe(this, &EditorApp::onActiveNodeChanged, [&]() {
            rebuildInspector();
    });
}

nbui::LayoutBuilder EditorApp::buildFieldUI(
    nbui::LayoutBuilder parentBuilder,
    void* componentPtr,
    const nb::Reflect::TypeInfo* info,
    const nb::Reflect::FieldInfo& field
) noexcept
{
    using namespace nbui;

    if (std::string(field.name) == "dirty")
    {
        return parentBuilder;
    }

    if (field.visibleIf && !field.visibleIf(componentPtr))
    {
        return parentBuilder;
    }

    void* fieldData = (char*)componentPtr + field.offset;
    std::string typeName = field.type->name;

    if (field.type->isEnum)
    {
        auto row = LayoutBuilder::hBox().relativeWidth(1.0f).absoluteHeight(30);

        // 1. Название поля
        row = std::move(row).child(
            LayoutBuilder::label(nb::Utils::toWString(field.name))
                .relativeWidth(0.35f)
                .color({180, 180, 180})
                .textAlignment({.textAlignment = TextAlignment::LEFT})
        );

        // 2. Выпадающий список
        row = std::move(row).child(
            LayoutBuilder::widget(new Widgets::ComboBox())
                .relativeWidth(0.65f)
                .absoluteHeight(25)
                .background({45, 45, 45})
                .apply<Widgets::ComboBox>(
                    [fieldData, componentPtr, info, field, this](Widgets::ComboBox* cb)
                    {
                        for (const auto& enumName : field.type->enumValues)
                        {
                            cb->addItem({nb::Utils::toWString(enumName.name), enumName.value});
                        }

                        int currentVal = *static_cast<int*>(fieldData);
                        cb->setSelectedItem(currentVal);
                    }
                )
                .onEvent(&Widgets::ComboBox::onSelectionChanged, [&, fieldData, componentPtr, info, this](const Widgets::ListItem& item)
                {
                    *static_cast<int*>(fieldData) = item.getValue<int>();

                    markComponentDirty(componentPtr, info);
                    
                    shouldRebuildInspector = true;
                })
        );

        return std::move(parentBuilder).child(std::move(row));
    }
    else if (typeName.find("Vector3") != std::string::npos ||
        typeName.find("Quaternion") != std::string::npos)
    {
        auto row = LayoutBuilder::hBox().relativeWidth(1.0f).absoluteHeight(30);

        row = std::move(row).child(
            LayoutBuilder::label(nb::Utils::toWString(field.name))
                .relativeWidth(0.35f)
                .color({180, 180, 180})
                .textAlignment({.textAlignment = TextAlignment::LEFT})

        );

        auto fieldsBox = LayoutBuilder::hBox().relativeWidth(0.65f);
        NbColor colors[] = {{180, 40, 40}, {40, 160, 40}, {40, 40, 180}};

        for (int i = 0; i < 3 && i < (int)field.type->fields.size(); ++i)
        {
            auto& subField = field.type->fields[i];
            void* subFieldData = (char*)fieldData + subField.offset;

            fieldsBox = std::move(fieldsBox).child(
                LayoutBuilder::vBox().relativeWidth(0.33f).child(
                    LayoutBuilder::widget(new Widgets::FloatSpinBox())
                        .apply<Widgets::FloatSpinBox>(
                            [subFieldData, componentPtr, info, this, field](Widgets::FloatSpinBox* c)
                            {
                                c->setRange(-1000, 1000);
                                c->setStep(field.step);
                                c->bind(
                                    [subFieldData]()
                                    {
                                        return *static_cast<float*>(subFieldData);
                                    },
                                    [subFieldData, componentPtr, info, this](float v)
                                    {
                                        *static_cast<float*>(subFieldData) = v;
                                        markComponentDirty(componentPtr, info);
                                        
                                    }
                                );
                            }
                        )
                        .relativeWidth(1.0f)
                        .absoluteHeight(30)
                        .background({25, 25, 25})
                        .color(colors[i])
                )
            );
        }

        row = std::move(row).child(std::move(fieldsBox));
        return std::move(parentBuilder).child(std::move(row));
    }
    else if (typeName == "Color")
    {
        nb::Color* colorPtr = static_cast<nb::Color*>(fieldData);

        auto colorRow =
            LayoutBuilder::hBox()
                .relativeWidth(1.0f)
                .absoluteHeight(30)
                .child(
                    LayoutBuilder::label(nb::Utils::toWString(field.name))
                        .relativeWidth(0.35f)
                        .color({180, 180, 180})
                        .textAlignment({.textAlignment = TextAlignment::LEFT})

                )
                .child(
                    LayoutBuilder::widget(new Widgets::Button())
                        .relativeWidth(0.65f)
                        .absoluteHeight(25)
                        .background(NbColor(colorPtr->toRgb().r, colorPtr->toRgb().g, colorPtr->toRgb().b
                        ))
                        .apply<Widgets::Button>(
                            [this, colorPtr, componentPtr, info](Widgets::Button* btn)
                            {
                                btn->setText(L"");

                                subscribe(
                                    btn, &Widgets::Button::onReleasedSignal,
                                    [this, colorPtr, btn, componentPtr, info]()
                                    {
                                        // 1. ГАРАНТИРОВАННОЕ ПЕРЕСОЗДАНИЕ:
                                        // Если старое окно было, сбрасываем его.
                                        // Но внимание: если мы в модальном цикле, старое окно
                                        // должно быть уже закрыто.
                                        if (colorPickerWindow)
                                        {
                                            colorPickerWindow = nullptr;
                                        }

                                        // 2. Создаем новое окно и сохраняем его в локальную
                                        // переменную
                                        NbSize<int> size = {300, 600}; // 400?

                                        auto newWin = std::make_shared<Win32Window::ModalWindow>(
                                            size,
                                            inspectorWindow.get()
                                        );
                                        colorPickerWindow = newWin; // Сохраняем в член класса

                                        newWin->setTitle(L"Color Picker");

                                        auto ui =
                                            LayoutBuilder::vBox()
                                                .style(
                                                    [](auto& s)
                                                    {
                                                        s.color = {45, 45, 45};
                                                    }
                                                )
                                                .child(
                                                    LayoutBuilder::widget(
                                                        new Widgets::ColorPicker({0, 0, 300, 500})
                                                    )
                                                        .relativeHeight(1.0f)
                                                        .relativeWidth(1.0f)
                                                        .apply<Widgets::ColorPicker>(
                                                            [colorPtr](Widgets::ColorPicker* p)
                                                            {
                                                                p->setColor(*colorPtr);
                                                            }
                                                        )
                                                        .onEvent(
                                                            &Widgets::ColorPicker::onColorChanged,
                                                            [this, colorPtr, btn, componentPtr,
                                                             info,
                                                             newWin](const nb::Color& newColor)
                                                            {
                                                                // ФИКС nullptr: захватываем newWin
                                                                // (shared_ptr) по значению. Теперь,
                                                                // даже если кто-то обнулит
                                                                // this->colorPickerWindow, текущее
                                                                // окно и его виджеты будут жить,
                                                                // пока мы не выйдем из этой
                                                                // функции.

                                                                //*colorPtr = newColor;

                                                                
                                                            }
                                                        )
                                                        .onEvent(
                                                            &Widgets::ColorPicker::
                                                                onOkButtonPressed,
                                                            [this, colorPtr, btn, componentPtr,
                                                             info, newWin](const nb::Color& color)
                                                            {
                                                                *colorPtr = color;
                                                                if (btn)
                                                                {
                                                                    btn->setColor(NbColor(
                                                                        color.toRgb().r,
                                                                        color.toRgb().g,
                                                                        color.toRgb().b
                                                                    ));
                                                                }

                                                                PostMessage(
                                                                    (HWND)newWin->getHandle()
                                                                        .as<HWND>(),
                                                                    WM_CLOSE, 0, 0
                                                                );

                                                                markComponentDirty(
                                                                    componentPtr, info
                                                                );
                                                            }
                                                        )
                                                        .onEvent(
                                                            &Widgets::ColorPicker::
                                                                onCancelButtonPressed,
                                                            [this, newWin]()
                                                            {
                                                                PostMessage(
                                                                    (HWND)newWin->getHandle()
                                                                        .as<HWND>(),
                                                                    WM_CLOSE, 0, 0
                                                                );
                                                            }
                                                        )
                                                )
                                                .build();

                                        newWin->getLayoutRoot()->addChild(std::move(ui));
                                        newWin->show();
                                    }
                                );
                            }
                        )
                );

        return std::move(parentBuilder).child(std::move(colorRow));
    }
    else if (typeName == "float")
    {
        auto floatRow = LayoutBuilder::hBox()
                            .relativeWidth(1.0f)
                            .absoluteHeight(30)
                            .child(
                                LayoutBuilder::label(nb::Utils::toWString(field.name))
                                    .relativeWidth(0.35f)
                                    .color({180, 180, 180})
                                    .textAlignment({.textAlignment = TextAlignment::LEFT})

                            )
                            .child(
                                LayoutBuilder::widget(new Widgets::FloatSpinBox())
                                    .apply<Widgets::FloatSpinBox>(
                                        [fieldData, componentPtr, info,
                                         this, field](Widgets::FloatSpinBox* c)
                                        {
                                            c->setStep(field.step);
                                            c->bind(
                                                [fieldData]()
                                                {
                                                    return *static_cast<float*>(fieldData);
                                                },
                                                [fieldData, componentPtr, info, this](float v)
                                                {
                                                    *static_cast<float*>(fieldData) = v;
                                                    markComponentDirty(componentPtr, info);
                                                }
                                            );
                                        }
                                    )
                                    .relativeWidth(0.65f)
                                    .absoluteHeight(30)
                                    .background({25, 25, 25})
                            );

        return std::move(parentBuilder).child(std::move(floatRow));
    }
    else if (typeName == "bool")
    {
        auto boolRow = LayoutBuilder::hBox()
                           .relativeWidth(1.0f)
                           .absoluteHeight(30)
                           .child(
                               LayoutBuilder::label(nb::Utils::toWString(field.name))
                                   .relativeWidth(0.35f)
                                   .color({180, 180, 180})
                                   .textAlignment({.textAlignment = TextAlignment::LEFT})
                           )
                           .child(
                               LayoutBuilder::widget(new Widgets::CheckBox())
                                   .apply<Widgets::CheckBox>(
                                       [fieldData, componentPtr, info, this](Widgets::CheckBox* c)
                                       {
                                           c->setChecked(*static_cast<bool*>(fieldData));
                                           c->onToggled(
                                               [fieldData, componentPtr, info, this](bool value)
                                               {
                                                   *static_cast<bool*>(fieldData) = value;
                                                   markComponentDirty(componentPtr, info);
                                               }
                                           );
                                       }
                                   )
                                   .relativeWidth(0.65f)
                                   .absoluteHeight(30)
                           );

        return std::move(parentBuilder).child(std::move(boolRow));
    }
    else if (typeName.find("std::vector<Ref<nb::Resource::MaterialAsset>>") != std::string::npos)
    {
        using MaterialVec   = std::vector<Ref<nb::Resource::MaterialAsset>>;
        MaterialVec* vecPtr = static_cast<MaterialVec*>(fieldData);

        // Параметры верстки
        const int slotHeight   = 50; 
        const int headerHeight = 30; 

        auto vectorColumn = LayoutBuilder::vBox().relativeWidth(1.0f).absoluteHeight(
            (int)vecPtr->size() * slotHeight + headerHeight
        );

        vectorColumn = std::move(vectorColumn)
                           .child(
                               LayoutBuilder::label(nb::Utils::toWString(field.name))
                                   .relativeWidth(1.0f)
                                   .absoluteHeight(headerHeight)
                                   .color({150, 150, 150})
                                   .textAlignment({.textAlignment = TextAlignment::LEFT})
                           );

        for (size_t i = 0; i < vecPtr->size(); ++i)
        {
            auto& materialRef = (*vecPtr)[i];

            std::wstring fullPath =
                materialRef ? nb::Utils::toWString(materialRef->getPath()) : L"None";
            std::wstring fileName  = fullPath;
            size_t       lastSlash = fileName.find_last_of(L"/\\");
            if (lastSlash != std::wstring::npos)
            {
                fileName = fileName.substr(lastSlash + 1);
            }

            vectorColumn =
                std::move(vectorColumn)
                    .child(
                        LayoutBuilder::hBox()
                            .relativeWidth(1.0f)
                            .absoluteHeight(slotHeight)
                            .margin({0, 2, 0, 2}) 
                            .child(
                                LayoutBuilder::label(L" Slot " + std::to_wstring(i))
                                    .relativeWidth(0.35f)
                                    .color({100, 100, 100})
                            )
                            .child(
                                LayoutBuilder::widget(new Widgets::MaterialWidget())
                                    .relativeWidth(0.65f)
                                    .absoluteHeight(slotHeight)
                                    .background({45, 45, 45})
                                    .apply<Widgets::MaterialWidget>(
                                        [fileName, materialRef](Widgets::MaterialWidget* w)
                                        {
                                            w->setMaterial(fileName, materialRef != nullptr);
                                        }
                                    )
                                   
                            )
                    );
        }

        return std::move(parentBuilder).child(std::move(vectorColumn));
    }
    else if (field.getResourcePath)
    {
        std::string  path     = field.getResourcePath(fieldData);
        std::wstring fileName = L"None";
        if (!path.empty())
        {
            size_t lastSlash = path.find_last_of("/\\");
            fileName         = nb::Utils::toWString(
                lastSlash == std::string::npos ? path : path.substr(lastSlash + 1)
            );
        }

        auto resourceRow =
            LayoutBuilder::hBox()
                .relativeWidth(1.0f)
                .absoluteHeight(35)
                //.alignment(Alignment::CENTER_LEFT)
                .child(
                    LayoutBuilder::label(nb::Utils::toWString(field.name))
                        .relativeWidth(0.35f)
                        .color({180, 180, 180})
                        .textAlignment({.textAlignment = TextAlignment::LEFT})
                )
                .child(
                    LayoutBuilder::hBox()
                        .relativeWidth(0.65f)
                        .absoluteHeight(30)
                        //.alignment(Alignment::CENTER_LEFT)
                        .child(
                            LayoutBuilder::vBox()
                                .absoluteWidth(35)
                                .absoluteHeight(35)
                                .background({70, 140, 240}) 
                                //.cornerRadius(5.0f)         // Скругление (аккуратное)
                                .margin({2, 2, 2, 2})       
                        )
                        .child(
                            // 2. САМА КНОПКА (Widgets::Button)
                            LayoutBuilder::widget(new Widgets::Button())
                                .relativeWidth(1.0f)
                                .absoluteHeight(31)
                                .background({50, 50, 50})
                                .margin({2,2,2,2})
                                .apply<Widgets::Button>(
                                    [fileName](Widgets::Button* btn)
                                    {
                                        std::wstring buttonText = fileName + L"   🔍";
                                        btn->setText(buttonText);
                                    }
                                )
                                .onEvent(
                                    &Widgets::IWidget::onReleasedSignal,
                                    [this, field, fieldData, componentPtr, info]()
                                    {
                                        if (filePickerWindow)
                                        {
                                            filePickerWindow = nullptr;
                                        }
                                        NbSize<int> winSize = {500, 600};
                                        auto newWin = std::make_shared<Win32Window::ModalWindow>(
                                            winSize, inspectorWindow.get()
                                        );
                                        filePickerWindow = newWin;
                                        newWin->setTitle(
                                            L"Select Resource: " + nb::Utils::toWString(field.name)
                                        );

                                        auto resourceLoader = field.loadResource;
                                        std::weak_ptr<Win32Window::ModalWindow> weakWin = newWin;

                                        auto ui =
                                            LayoutBuilder::vBox()
                                                .style(
                                                    [](auto& s)
                                                    {
                                                        s.color = {40, 40, 40};
                                                    }
                                                )
                                                .child(
                                                    LayoutBuilder::widget(
                                                        new Widgets::FilePicker({0, 0, 500, 600})
                                                    )
                                                        .relativeWidth(1.0f)
                                                        .relativeHeight(1.0f)
                                                        .onEvent(
                                                            &Widgets::FilePicker::onFileSelected,
                                                            [this, resourceLoader, fieldData,
                                                             componentPtr, info,
                                                             weakWin](const std::string& newPath)
                                                            {
                                                                if (auto pinnedWin = weakWin.lock())
                                                                {
                                                                    if (!newPath.empty() &&
                                                                        resourceLoader)
                                                                    {
                                                                        resourceLoader(
                                                                            fieldData, newPath
                                                                        );
                                                                        markComponentDirty(
                                                                            componentPtr, info
                                                                        );
                                                                        shouldRebuildInspector =
                                                                            true;
                                                                    }
                                                                    PostMessage(
                                                                        (HWND)pinnedWin->getHandle()
                                                                            .as<HWND>(),
                                                                        WM_CLOSE, 0, 0
                                                                    );
                                                                }
                                                            }
                                                        )
                                                        .onEvent(
                                                            &Widgets::FilePicker::
                                                                onCancelButtonPressed,
                                                            [weakWin]()
                                                            {
                                                                if (auto pinnedWin = weakWin.lock())
                                                                {
                                                                    PostMessage(
                                                                        (HWND)pinnedWin->getHandle()
                                                                            .as<HWND>(),
                                                                        WM_CLOSE, 0, 0
                                                                    );
                                                                }
                                                            }
                                                        )
                                                )
                                                .build();
                                        newWin->getLayoutRoot()->addChild(std::move(ui));
                                        newWin->show();
                                    }
                                )
                        )
                );

        return std::move(parentBuilder).child(std::move(resourceRow));
    }

    return parentBuilder;
}

void EditorApp::addCubeToEntity(
    const Widgets::ModelIndex& index,
    Widgets::TreeView*         tv
) noexcept
{
    if (!index.isValid())
    {
        return;
    }

    auto* item = sceneModel->findById(index.getUuid());
    if (!item)
    {
        return;
    }

    auto  parentId = reinterpret_cast<nb::Ecs::EntityID>(item->getData());
    auto& scene    = nb::Scene::getInstance();

    auto entity = scene.createNode(parentId);
    entity.addComponent<MeshComponent>(
        {.mesh = nb::Renderer::PrimitiveGenerators::createCube(), .material = nullptr}
    );
    entity.addComponent<TransformComponent>({});

    sceneModel->addEntity(parentId, entity.getId());
    tv->refresh();

    activeNode = scene.getNode(entity.getId());
    onActiveNodeChanged.emit();
    scene.invalidateBvh();
}

void EditorApp::addSphereToEntity(
    const Widgets::ModelIndex& index,
    Widgets::TreeView*         tv
) noexcept
{
    if (!index.isValid())
    {
        return;
    }

    auto* item = sceneModel->findById(index.getUuid());
    if (!item)
    {
        return;
    }

    auto  parentId = reinterpret_cast<nb::Ecs::EntityID>(item->getData());
    auto& scene    = nb::Scene::getInstance();

    auto entity = scene.createNode(parentId);
    entity.addComponent<MeshComponent>(
        {.mesh = nb::Renderer::PrimitiveGenerators::createSphere(1.0f, 32, 32), .material = nullptr}
    );
    entity.addComponent<TransformComponent>({});

    sceneModel->addEntity(parentId, entity.getId());
    tv->refresh();

    activeNode = scene.getNode(entity.getId());
    onActiveNodeChanged.emit();
    scene.invalidateBvh();
}

void EditorApp::spawnPrimitive(
    const Widgets::ModelIndex& index,
    void*                      data,
    nb::Reflect::TypeInfo*     typeInfo
) noexcept
{
    if (!index.isValid() || !data || !typeInfo)
    {
        return;
    }

    auto* item = sceneModel->findById(index.getUuid());
    if (!item)
    {
        return;
    }

    auto  parentId = reinterpret_cast<nb::Ecs::EntityID>(item->getData());
    auto& scene    = nb::Scene::getInstance();

    auto node = scene.createNode(parentId);

    std::shared_ptr<nb::Renderer::Mesh>     mesh;
    nb::Renderer::Mesh::PrimitiveDescriptor desc;
    desc.type = typeInfo->name;

    for (auto& field : typeInfo->fields)
    {
        void* fieldPtr = (uint8_t*)data + field.offset;

        if (strcmp(field.type->name, "float") == 0)
        {
            desc.parameters[field.name] = *static_cast<float*>(fieldPtr);
        }
        else if (strcmp(field.type->name, "int") == 0 || strcmp(field.type->name, "uint32_t") == 0)
        {
            desc.parameters[field.name] = static_cast<float>(*static_cast<int*>(fieldPtr));
        }
    }

    if (strcmp(typeInfo->name, "SphereParams") == 0)
    {
        auto* p = static_cast<SphereParams*>(data);
        mesh =
            nb::Renderer::PrimitiveGenerators::createSphere(p->radius, p->xSegments, p->ySegments);
    }
    else if (strcmp(typeInfo->name, "TorusParams") == 0)
    {
        auto* p = static_cast<TorusParams*>(data);
        mesh    = nb::Renderer::PrimitiveGenerators::createTorus(
            {(float)p->xSegments, (float)p->ySegments},
            p->majorRadius,
            p->minorRadius
        );
    }
    else if (strcmp(typeInfo->name, "CylinderParams") == 0)
    {
        auto* p = static_cast<CylinderParams*>(data);
        mesh    = nb::Renderer::PrimitiveGenerators::createCylinder(
            p->radius, p->height, p->xSegments, p->ySegments
        );
    }
    else if (strcmp(typeInfo->name, "PlaneParams") == 0)
    {
        auto* p = static_cast<PlaneParams*>(data);
        mesh    = nb::Renderer::PrimitiveGenerators::createPlane(
            p->width, p->height, p->xSegments, p->ySegments
        );
    }
    else if (strcmp(typeInfo->name, "ConeParams") == 0)
    {
        auto* p = static_cast<ConeParams*>(data);
        mesh    = nb::Renderer::PrimitiveGenerators::createCone(
            p->radius, p->height, p->radialSegments, p->heightSegments
        );
    }
    else if (strcmp(typeInfo->name, "PyramidParams") == 0)
    {
        auto* p = static_cast<PyramidParams*>(data);
        mesh    = nb::Renderer::PrimitiveGenerators::createPyramid(
            p->radius, p->height, p->sides
        );
    }



    if (mesh)
    {

        node.addComponent<MeshComponent>({.mesh = mesh, .material = nullptr});
        node.addComponent<TransformComponent>({});

        sceneModel->addEntity(parentId, node.getId());
        
        
        //tv->refresh();

        activeNode = scene.getNode(node.getId());
        onActiveNodeChanged.emit();
        scene.invalidateBvh();
    }
}


void EditorApp::setupHierarchyEvents(Widgets::TreeView* tv) noexcept
{
    using namespace nbui;

    subscribe(
        tv, &Widgets::TreeView::onItemClickSignal,
        [this](const auto& index)
        {
            if (index.isValid())
            {
                if (auto* item = sceneModel->findById(index.getUuid()))
                {
                    auto id    = reinterpret_cast<nb::Ecs::EntityID>(item->getData());
                    activeNode = nb::Scene::getInstance().getNode(id);
                    onActiveNodeChanged.emit();
                }
            }
        }
    );

    subscribe(
        tv, &Widgets::TreeView::onItemRightClickSignal,
        [this, tv](const auto& index)
        {
            auto popup = new PopupMenu();

            popup->addItem(
                L"➕ Добавить куб",
                [this, index, tv]()
                {
                    auto dialog = new PrimitiveCreationDialog(
                        inspectorWindow.get(),
                        "SphereParams", 
                        [this, index](void* data, nb::Reflect::TypeInfo* typeInfo)
                        {
                            this->spawnPrimitive(index, data, typeInfo);
                        }
                    );
                    dialog->show();
                    //m_activeDialog = dialog; 
                    //this->addCubeToEntity(index, tv);
                }
            );

            popup->addItem(
                L"➕ Добавить сферу",
                [this, index, tv]()
                {
                    this->addSphereToEntity(index, tv);
                }
            );

            popup->addItem(
                L"➕ Добавить тор",
                [this, index, tv]()
                {
                    auto dialog = new PrimitiveCreationDialog(
                        inspectorWindow.get(), "TorusParams",
                        [this, index](void* data, nb::Reflect::TypeInfo* typeInfo)
                        {
                            this->spawnPrimitive(index, data, typeInfo);
                        }
                    );
                    dialog->show();
                    // m_activeDialog = dialog;
                }
            );

            popup->addItem(
                L"➕ Добавить цилиндр",
                [this, index, tv]()
                {
                    auto dialog = new PrimitiveCreationDialog(
                        inspectorWindow.get(), "CylinderParams",
                        [this, index](void* data, nb::Reflect::TypeInfo* typeInfo)
                        {
                            this->spawnPrimitive(index, data, typeInfo);
                        }
                    );
                    dialog->show();
                    // m_activeDialog = dialog;
                }
            );

            popup->addItem(
                L"➕ Добавить плоскость",
                [this, index, tv]()
                {
                    auto dialog = new PrimitiveCreationDialog(
                        inspectorWindow.get(), "PlaneParams",
                        [this, index](void* data, nb::Reflect::TypeInfo* typeInfo)
                        {
                            this->spawnPrimitive(index, data, typeInfo);
                        }
                    );
                    dialog->show();
                    // m_activeDialog = dialog;
                }
            );

            popup->addItem(
                L"➕ Добавить конус",
                [this, index, tv]()
                {
                    auto dialog = new PrimitiveCreationDialog(
                        inspectorWindow.get(), "ConeParams",
                        [this, index](void* data, nb::Reflect::TypeInfo* typeInfo)
                        {
                            this->spawnPrimitive(index, data, typeInfo);
                        }
                    );
                    dialog->show();
                    // m_activeDialog = dialog;
                }
            );

            popup->addItem(
                L"➕ Добавить пирамиду",
                [this, index, tv]()
                {
                    auto dialog = new PrimitiveCreationDialog(
                        inspectorWindow.get(), "PyramidParams",
                        [this, index](void* data, nb::Reflect::TypeInfo* typeInfo)
                        {
                            this->spawnPrimitive(index, data, typeInfo);
                        }
                    );
                    dialog->show();
                    // m_activeDialog = dialog;
                }
            );

            popup->addItem(
                L"✏️ Переименовать",
                [tv, index]()
                {
                    tv->startEditing(index);
                }
            );

            //popup->addSeparator();

            popup->addItem(
                L"🗑️ Удалить",
                []()
                {
                    nb::Error::ErrorManager::instance().report(
                        nb::Error::Type::INFO, "Delete requested"
                    );
                }
            );

            auto mousePos = this->mainWindow->getMousePosition();
            this->hierarchyWindow->getPopupManager().show(popup, mousePos.x, mousePos.y);
        }
    );
}


void EditorApp::markComponentDirty(
    void* componentPtr,
    const nb::Reflect::TypeInfo* typeInfo
) noexcept
{
    if (!componentPtr || !typeInfo)
    {
        return;
    }

    for (const auto& field : typeInfo->fields)
    {
        if (std::string(field.name) == "dirty" || std::string(field.name) == "physicsDirty")
        {
            void* bytePtr = static_cast<char*>(componentPtr) + field.offset;
            bool* dirtyPtr = reinterpret_cast<bool*>(bytePtr);

            *dirtyPtr = true;
            return;
        }
    }
}
