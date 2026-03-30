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

#include <Renderer/Shader.hpp>
#include <Renderer/Material.hpp>
#include <Renderer/Texture.hpp>
#include <Renderer/Mesh.hpp>

#include <Renderer/Objects/Objects.hpp>
#include <Renderer/Scene.hpp>
#include <memory>




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
    mainWindow->excludeFromClientRect({32, 0, 0, 0});


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

    // textureInspector = std::make_shared<Win32Window::ChildWindow>(mainWindow.get(), true);
    // textureInspector->setTitle(
    //     Utils::toWstring(Translation::fromKey("Ui.Editor.TextureView.Title"))
    // );

    debugWindow = std::make_shared<Win32Window::ChildWindow>(mainWindow.get());
    debugWindow->setTitle(
        Utils::toWstring(Translation::fromKey("Ui.Editor.DebugWindow.Title"))
    );

    

}

void EditorApp::setupDocking() noexcept
{
    dockManager = std::make_unique<Temp::DockingSystem>(mainWindow);

    auto sceneTab = dockManager->dockAsTab(sceneWindow, nullptr, "Scene");
    dockManager->dockRelative(
        hierarchyWindow,
        Temp::DockPosition::LEFT,
        sceneTab->getWindow(),
        Temp::Percent(25)
    );

    dockManager->dockRelative(
        inspectorWindow,
        Temp::DockPosition::RIGHT,
        sceneTab->getWindow(),
        Temp::Percent(20)
    );

    dockManager->dockRelative(
        debugWindow,
        Temp::DockPosition::BOTTOM,
        inspectorWindow,
        Temp::Percent(60)
    );

    dockManager->dockRelative(
         assetManager,
         Temp::DockPosition::BOTTOM,
         debugWindow,
         Temp::Percent(40)
    );

    
    subscribe(*mainWindow, &Win32Window::Window::onRectChanged, [this](const NbRect<int>& rect) {
        dockManager->onSize(rect.width, rect.height);
    });

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
                    s.width = 1.0f;
                    s.height = 1.0f;
                    s.widthSizeType = NNsLayout::SizeType::RELATIVE;
                    s.heightSizeType = NNsLayout::SizeType::RELATIVE;
                    s.color = NbColor{30, 30, 30};
                }
            )
            .child(
                LayoutBuilder::toolbar()
                    .buttonGroupOnlyOne()
                    .relativeHeight(1.0f)
                    .absoluteWidth(120)
                    .child(
                        LayoutBuilder::widget(new Widgets::Button())
                            .text(L"2D")
                            .absoluteWidth(50)
                            .relativeHeight(1.0f)
                    )
                    .child(
                        LayoutBuilder::widget(new Widgets::Button())
                            .text(L"3D")
                            .absoluteWidth(50)
                            .relativeHeight(1.0f)
                    )

                    .endGroup()

                    .buttonGroupMultiple()
                    .relativeHeight(1.0f)
                    .relativeWidth(0.5f)
                    .child(
                        LayoutBuilder::widget(new Widgets::Button())
                            .text(L"R")
                            .absoluteWidth(50)
                            .relativeHeight(1.0f)
                            .background({180, 60, 60}, LayoutBuilder::StateStyle::ACTIVE)
                            .onEvent(
                                &Widgets::Button::onCheckedChangedSignal,
                                [this](bool flag)
                                {
                                    //settings.channelMask.x = flag ? 1.0f : 0.0f;
                                    //onRender();
                                }
                            )
                    )
                    .child(
                        LayoutBuilder::widget(new Widgets::Button())
                            .text(L"G")
                            .absoluteWidth(50)
                            .relativeHeight(1.0f)
                            .background({60, 150, 80}, LayoutBuilder::StateStyle::ACTIVE)
                            .onEvent(
                                &Widgets::Button::onCheckedChangedSignal,
                                [this](bool flag)
                                {
                                    //settings.channelMask.y = flag ? 1.0f : 0.0f;
                                    //onRender();
                                }
                            )
                    )
                    .child(
                        LayoutBuilder::widget(new Widgets::Button())
                            .text(L"B")
                            .absoluteWidth(50)
                            .relativeHeight(1.0f)
                            .background({60, 100, 180}, LayoutBuilder::StateStyle::ACTIVE)
                            .onEvent(
                                &Widgets::Button::onCheckedChangedSignal,
                                [this](bool flag)
                                {
                                    //settings.channelMask.z = flag ? 1.0f : 0.0f;
                                    //onRender();
                                }
                            )
                    )
                    .child(
                        LayoutBuilder::widget(new Widgets::Button())
                            .text(L"A")
                            .absoluteWidth(50)
                            .relativeHeight(1.0f)
                            .background({150, 150, 150}, LayoutBuilder::StateStyle::ACTIVE)

                    )
                    .checkedGroupIndex(true, 0)
                    .checkedGroupIndex(true, 1)
                    .checkedGroupIndex(true, 2)
                    .checkedGroupIndex(true, 3)

                    .endGroup()

            )
            .build();

    mainWindow->getLayoutRoot()->addChild(std::move(rootUI));
}

void EditorApp::setupHierarchyUI() noexcept
{
    using namespace nbui;
    auto ui = LayoutBuilder::vBox()
        .style([](auto& s) { 
                s.padding = { 10, 10, 10, 10 };
                s.color = { 30, 30, 30 };
            }
        )
        .child(LayoutBuilder::treeView()
            .apply<Widgets::TreeView>([&, this](auto* tv)
                { 
                    tv->setModel(sceneModel);

                    auto popup = std::make_unique<nbui::PopupMenu>();
                    popup->addItem(
                        L"Удалить",
                        [&]()
                        {
                           
                        }
                    );
                    popup->addItem(L"Копировать", []() {  });

                    mainWindow->attachMenuToWidget(tv, popup.get());
                    subscribe(
                        tv, &Widgets::TreeView::onItemRightClickSignal,
                        [&, tv](const Widgets::ModelIndex& index)
                        {
                            auto popup = new nbui::PopupMenu();

                            popup->addItem(
                                L"Удалить",
                                []()
                                {
                                    nb::Error::ErrorManager::instance().report(
                                        nb::Error::Type::INFO, "CLICKED"
                                    );

                                    
                                }
                            );

                            

                             popup->addItem(
                                L"Добавить куб",
                                [this, index, tv]()
                                {
                                    if (!index.isValid())
                                    {
                                        return;
                                    }

                                    auto* parentItem = sceneModel->findById(index.getUuid());
                                    if (!parentItem)
                                    {
                                        return;
                                    }

                                    auto& scene = nb::Scene::getInstance();

                                    auto parentEntity =
                                        reinterpret_cast<nb::Ecs::EntityID>(parentItem->getData());

                                    auto entity = scene.createNode(parentEntity);
                                    entity.addComponent<MeshComponent>({
                                        .mesh = nb::Renderer::PrimitiveGenerators::createCube(),
                                        .material = nullptr
                                    });
                                    entity.addComponent<TransformComponent>({});
                                
                                    sceneModel->addEntity(parentEntity, entity.getId());

                                    tv->refresh();
                                    onActiveNodeChanged.emit();
                                }
                            );

                             

                             popup->addItem(
                                 L"Переименовать",
                                 [tv, index]()
                                 {
                                     tv->startEditing(index);
                                 }
                             );
                           

                            auto pos = this->mainWindow->getMousePosition();

                            this->hierarchyWindow->getPopupManager().show(popup, pos.x, pos.y);
                        }
                    );
                }
            )
            .onEvent(&Widgets::TreeView::onItemClickSignal, [this](const auto& index) {
                if (index.isValid())
                {
                    activeNode = 
                        nb::Scene::getInstance().getNode(
                            reinterpret_cast<nb::Ecs::EntityID>(sceneModel->findById(index.getUuid())->getData()
                    )
                    
                    );
                    onActiveNodeChanged.emit();
                }
            })
            .relativeHeight(1.0f)
            .relativeWidth(1.0f)
             
             
            
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
    //materialEditor = std::make_shared<MaterialEditor>(
    //    debugWindow.get(), engine.get(),
    //    res->getResource<nb::Resource::MaterialAsset>("Assets/res/plastic.material").get()
    //);
    //materialEditor->show();
    // 
    // 
    //textureEditor = std::make_shared<TextureEditor>(debugWindow.get(), engine.get(), res->getResource<nb::Resource::TextureAsset>("Assets/res/normal.texture").get());
    // auto* resMan = nb::ResMan::ResourceManager::getInstance();
    //textureEditor->show();
    // // 1. Создаем модальное окно (сохраняем в поле класса, чтобы не удалилось)
    // modal =
    //     std::make_shared<Win32Window::ModalWindow>(NbSize<int>{1100, 700}, debugWindow.get());
    // modal->setTitle(L"Content Browser");

    // modalTexturePreview = std::make_shared<Win32Window::ChildWindow>(modal.get(), true);
    // modalTexturePreview->setTitle(L"Preview Texture");

    // subscribe(*modal, &Win32Window::ModalWindow::onSizeChanged, [this](const NbSize<int>& size) {
    //         modalTexturePreview->setSize({static_cast<int>(size.width * 0.7f), size.height - 32});
    // });

    // modalTexturePreview->setSize(
    //     {static_cast<int>(modal->getClientSize().width * 0.7f), modal->getClientSize().height - 32}
    // );
    
    // modalTexturePreview->setPosition({5, 32 + 35});
    // sharedCtx = engine->getRenderer()->createSharedContextForWindow(modalTexturePreview->getHandle().as<HWND>());
    // modalTexturePreview->setRenderable(false);


    // modalInspector = std::make_shared<Win32Window::ChildWindow>(modal.get());
    // modalInspector->setTitle(L"Inspector");

    // const NbSize<int>& modalSize = modal->getClientSize();
    // const NbSize<int>& texturePreviewSize = modalTexturePreview->getClientSize();

    // subscribe(*modal, &Win32Window::ModalWindow::onSizeChanged, [this, &modalSize, &texturePreviewSize](const NbSize<int>& size) {
            
    //         modalInspector->setSize({modalSize.width - texturePreviewSize.width - 1, size.height - 32});
    //         modalInspector->setPosition({5 + texturePreviewSize.width, 32 + 35});
    //     });

    // modalInspector->setSize(
    //     {modalSize.width - texturePreviewSize.width, modalSize.height - 32}
    // );
    // modalInspector->setPosition({5 + texturePreviewSize.width , 32 + 35});


    // auto getFileName = [](const std::string& path)
    // {
    //     return std::filesystem::path(path).filename().wstring();
    // };

    // // 3. Создаем базовый FlowLayout для ассетов
    // auto assetFlow = LayoutBuilder::flow().style(
    //     [](NNsLayout::LayoutStyle& s)
    //     {
    //         s.width = 1.0f;
    //         s.widthSizeType = NNsLayout::SizeType::RELATIVE;
    //         s.heightSizeType = NNsLayout::SizeType::AUTO; // Растянется по количеству карточек
    //         s.padding = {10, 10, 10, 10};
    //         s.color = NbColor{20, 20, 20}; // Фон области контента
    //     }
    // );

    // // 4. НАПОЛНЯЕМ СЕТКУ ИЗ RESOURCE MANAGER

    // // Категория: ТЕКСТУРЫ (Зеленый индикатор)
    // //try
    // //{
    // //    auto& textures = resMan->getAllResources<nb::Renderer::Texture>();
    // //    for (auto const& [path, res] : textures)
    // //    {
    // //        assetFlow = std::move(assetFlow).child(createAssetCard(getFileName(path), L"Texture2D", NbColor{76, 175, 80}));
    // //    }
    // //}
    // //catch (...)
    // //{ /* Пул пуст или не создан */
    // //}

    // // Категория: МАТЕРИАЛЫ (Оранжевый индикатор)
    // //try
    // //{
    // //    auto& materials = resMan->getAllResources<nb::Resource::Material>();
    // //    for (auto const& [path, res] : materials)
    // //    {
    // //        assetFlow = std::move(assetFlow.child(createAssetCard(getFileName(path), L"Material", NbColor{255, 152, 0})));
    // //    }
    // //}
    // //catch (...)
    // //{
    // //}

    // // Категория: ШЕЙДЕРЫ (Фиолетовый индикатор)
    // try
    // {
    //     auto& shaders = resMan->getAllResources<nb::Renderer::Shader>();
    //     for (auto const& [path, res] : shaders)
    //     {
    //         assetFlow = std::move(assetFlow).child(createAssetCard(getFileName(path), L"Shader", NbColor{156, 39, 176}));
    //     }
    // }
    // catch (...)
    // {
    // }

    // // Категория: МЕШИ (Синий индикатор)
    // try
    // {
    //     auto& meshes = resMan->getAllResources<nb::Renderer::Mesh>();
    //     for (auto const& [path, res] : meshes)
    //     {
    //         assetFlow = std::move(assetFlow).child(
    //             createAssetCard(getFileName(path), L"StaticMesh", NbColor{33, 150, 243})
    //         );
    //     }
    // }
    // catch (...)
    // {
    // }


    // auto rootUI =
    //     LayoutBuilder::vBox()
    //         .style(
    //             [](NNsLayout::LayoutStyle& s)
    //             {
    //                 s.width = 1.0f;
    //                 s.height = 1.0f;
    //                 s.widthSizeType = NNsLayout::SizeType::RELATIVE;
    //                 s.heightSizeType = NNsLayout::SizeType::RELATIVE;
    //                 s.color = NbColor{30, 30, 30};
    //             }
    //         )
    //         .child(
    //             LayoutBuilder::toolbar()
    //                 .buttonGroupOnlyOne()
    //                           .relativeHeight(1.0f)
    //                           .absoluteWidth(120)
    //                           .child(
    //                               LayoutBuilder::widget(new Widgets::Button())
    //                                   .text(L"2D")
    //                                   .absoluteWidth(50)
    //                                   .relativeHeight(1.0f)
    //                           )
    //                           .child(
    //                               LayoutBuilder::widget(new Widgets::Button())
    //                                   .text(L"3D")
    //                                   .absoluteWidth(50)
    //                                   .relativeHeight(1.0f)
    //                           )
                              
    //                         .endGroup()

    //                     .buttonGroupMultiple()
    //                           .relativeHeight(1.0f)
    //                           .relativeWidth(0.5f)
    //                           .child(
    //                               LayoutBuilder::widget(new Widgets::Button())
    //                                   .text(L"R")
    //                                   .absoluteWidth(50)
    //                                   .relativeHeight(1.0f)
    //                                   .background({180, 60, 60}, LayoutBuilder::StateStyle::ACTIVE)
    //                                   .onEvent(&Widgets::Button::onCheckedChangedSignal, [this](bool flag) {
    //                                      texturePreviewRequest.channelMask.x = flag ? 1.0f : 0.0f;
    //                                     })
    //                           )
    //                           .child(
    //                               LayoutBuilder::widget(new Widgets::Button())
    //                                   .text(L"G")
    //                                   .absoluteWidth(50)
    //                                   .relativeHeight(1.0f)
    //                                   .background({60, 150, 80}, LayoutBuilder::StateStyle::ACTIVE)
    //                                     .onEvent(&Widgets::Button::onCheckedChangedSignal, [this](bool flag) {
    //                                         texturePreviewRequest.channelMask.y = flag ? 1.0f : 0.0f;
    //                                     })
    //                           )
    //                           .child(
    //                               LayoutBuilder::widget(new Widgets::Button())
    //                                   .text(L"B")
    //                                   .absoluteWidth(50)
    //                                   .relativeHeight(1.0f)     
    //                                   .background({60, 100, 180}, LayoutBuilder::StateStyle::ACTIVE)
    //                                     .onEvent(&Widgets::Button::onCheckedChangedSignal, [this](bool flag) {
    //                                         texturePreviewRequest.channelMask.z = flag ? 1.0f : 0.0f;
    //                                     })
    //                           )
    //                           .child(
    //                               LayoutBuilder::widget(new Widgets::Button())
    //                                   .text(L"A")
    //                                   .absoluteWidth(50)
    //                                   .relativeHeight(1.0f)
    //                                 .background({150, 150, 150}, LayoutBuilder::StateStyle::ACTIVE)

    //                           )
    //                         .checkedGroupIndex(true, 0)
    //                         .checkedGroupIndex(true, 1)
    //                         .checkedGroupIndex(true, 2)
    //                         .checkedGroupIndex(true, 3)

    //                         .endGroup()

    //                   )

                    
            
                

    //         //// Верхний Тулбар
    //         //.child(
    //         //    LayoutBuilder::hBox()
    //         //        .absoluteHeight(50)
    //         //        .background(NbColor{45, 45, 45})
                    
                    
            
    //         // Область со скроллом (наш FlowLayout)
    //         // .child(
    //         //     LayoutBuilder::vBox()
    //         //         .style(
    //         //             [](NNsLayout::LayoutStyle& s)
    //         //             {
    //         //                 s.width = 1.0f;
    //         //                 s.height = 1.0f;
    //         //                 s.widthSizeType = NNsLayout::SizeType::RELATIVE;
    //         //                 s.heightSizeType = NNsLayout::SizeType::RELATIVE;
    //         //             }
    //         //         )
    //         //         .child(std::move(assetFlow))       // Добавляем готовую сетку ассетов
    //         //         .child(LayoutBuilder::spacer()) // Прижимает сетку к верху
    //         // )
    //         // Статус-бар
    //         // .child(
    //         //     LayoutBuilder::hBox()
    //         //         .absoluteHeight(30)
    //         //         .relativeWidth(1.0f)
    //         //         .background(NbColor{20, 20, 20})
    //         //         .child(
    //         //             LayoutBuilder::label(L"  Total Resources Loaded: ...")
    //         //                 .fontSize(11)
    //         //                 .absoluteHeight(30)
    //         //                 .relativeWidth(1.0f)
    //         //         )
    //         // )
    //         .build();

    // // 6. Устанавливаем UI в окно и показываем
    // modal->getLayoutRoot()->addChild(std::move(rootUI));
    // modal->show();
    // //modalTexturePreview->setClientRect(});
    // modalTexturePreview->show();


    // auto inspectorUi =
    //     LayoutBuilder::vBox()
    //         .padding({10, 10, 0, 10})
    //         .relativeWidth(1.0f)  // Растягиваем на всю ширину окна инспектора
    //         .relativeHeight(1.0f) // Растягиваем на всю высоту

    //         // --- СЕКЦИЯ: ИНФОРМАЦИЯ ---
    //         .child(
    //             LayoutBuilder::vBox()
    //                 .relativeWidth(1.0f)
    //                 .autoHeight() // Высота всей секции метаданных
    //                 .child(
    //                     LayoutBuilder::label(L"METADATA")
    //                         .fontSize(12)
    //                         .color({150, 150, 150})
    //                         .absoluteHeight(20)
    //                         .relativeWidth(1.0f)
    //                 )
    //                 .child(
    //                     LayoutBuilder::label(L"Format: RGBA8_UNORM")
    //                         .fontSize(14)
    //                         .absoluteHeight(20)
    //                         .relativeWidth(1.0f)
    //                 )
    //                 .child(
    //                     LayoutBuilder::label(L"Size: 2048 x 2048")
    //                         .fontSize(14)
    //                         .absoluteHeight(20)
    //                         .relativeWidth(1.0f)
    //                 )
    //                 .child(
    //                     LayoutBuilder::label(L"Mips: 11")
    //                         .fontSize(14)
    //                         .absoluteHeight(20)
    //                         .relativeWidth(1.0f)
    //                 )
    //         )

    //         // Разделитель (фиксированная высота 2 пикселя)
    //         .child(
    //             LayoutBuilder::hBox()
    //                 .relativeWidth(1.0f)
    //                 .absoluteHeight(2)
    //                 .background({60, 60, 60})
    //                 .margin({10, 0, 10, 0}) // Отступы сверху и снизу
    //         )

    //         // --- СЕКЦИЯ: КАНАЛЫ (RGBA) ---
    //         .child(
    //             LayoutBuilder::vBox()
    //                 .relativeWidth(1.0f)
    //                 .absoluteHeight(60)
    //                 .child(
    //                     LayoutBuilder::label(L"CHANNELS")
    //                         .fontSize(12)
    //                         .color({150, 150, 150})
    //                         .absoluteHeight(20)
    //                         .relativeWidth(1.0f)
    //                 )
    //                 .child(
    //                     LayoutBuilder::hBox()
    //                         .relativeWidth(1.0f)
    //                         .absoluteHeight(35)
    //                         .buttonGroupMultiple()
    //                         .relativeWidth(1.0f)
    //                         .absoluteHeight(35)
    //                         .child(
    //                             LayoutBuilder::widget(new Widgets::Button())
    //                                 .text(L"R")
    //                                 .relativeWidth(0.25f)
    //                                 .relativeHeight(1.0f)
    //                                 .background({220, 80, 80}, LayoutBuilder::StateStyle::ACTIVE)
    //                         )
    //                         .child(
    //                             LayoutBuilder::widget(new Widgets::Button())
    //                                 .text(L"G")
    //                                 .relativeWidth(0.25f)
    //                                 .relativeHeight(1.0f)
    //                                 .background({110, 180, 120}, LayoutBuilder::StateStyle::ACTIVE)
    //                         )
    //                         .child(
    //                             LayoutBuilder::widget(new Widgets::Button())
    //                                 .text(L"B")
    //                                 .relativeWidth(0.25f)
    //                                 .relativeHeight(1.0f)
    //                                 .background({80, 140, 220}, LayoutBuilder::StateStyle::ACTIVE)
    //                         )
    //                         .child(
    //                             LayoutBuilder::widget(new Widgets::Button())
    //                                 .text(L"A")
    //                                 .relativeWidth(0.25f)
    //                                 .relativeHeight(1.0f)
    //                                 .background({210, 210, 210}, LayoutBuilder::StateStyle::ACTIVE)
    //                         )
    //                         .endGroup()
    //                 )
    //         )

    //         // Еще один разделитель
    //         .child(
    //             LayoutBuilder::hBox()
    //                 .relativeWidth(1.0f)
    //                 .absoluteHeight(2)
    //                 .background({60, 60, 60})
    //                 .margin({10, 0, 10, 0}) // Отступы сверху и снизу

    //         )

    //         // --- СЕКЦИЯ: ПАРАМЕТРЫ (Exposure, Gamma, Filtering) ---
    //         .child(
    //             LayoutBuilder::vBox()
    //                 .relativeWidth(1.0f)
    //                 .absoluteHeight(110) // Достаточно места для заголовка и 3 строк
    //                 .child(
    //                     LayoutBuilder::label(L"VISUALS")
    //                         .fontSize(12)
    //                         .color({150, 150, 150})
    //                         .absoluteHeight(20)
    //                         .relativeWidth(1.0f)
    //                 )
    //                 // Exposure
    //                 .child(
    //                     LayoutBuilder::hBox()
    //                         .relativeWidth(1.0f)
    //                         .absoluteHeight(30)
    //                         .child(
    //                             LayoutBuilder::label(L"Exposure")
    //                                 .relativeWidth(0.4f)
    //                                 .relativeHeight(1.0f)
    //                         )
    //                         .child(
    //                             LayoutBuilder::widget(new Widgets::Slider<float>())
    //                                  .apply<Widgets::Slider<float>>([this](auto* s) {
    //                                     s->setRange(0.0f, 10.0f, 0.1f);
    //                                     s->bind(
    //                                         [this]() { return texturePreviewRequest.exposure; },
    //                                         [this](float v) { texturePreviewRequest.exposure = v; }
    //                                     );
    //                                 })
    //                                 .relativeWidth(0.6f)
    //                                 .relativeHeight(1.0f)
    //                         )
    //                 )
    //                 // Gamma
    //                 .child(
    //                     LayoutBuilder::hBox()
    //                         .relativeWidth(1.0f)
    //                         .absoluteHeight(30)
    //                         .child(
    //                             LayoutBuilder::label(L"Gamma")
    //                             .relativeWidth(0.4f)
    //                             .relativeHeight(1.0f)
    //                         )
    //                         .child(
    //                             LayoutBuilder::widget(new Widgets::Slider<float>())
    //                                 .apply<Widgets::Slider<float>>(
    //                                     [this](Widgets::Slider<float>* c)
    //                                     {
    //                                         c->setRange(0.1f, 5.0f, 0.05f);

    //                                         // 2. Биндим данные
    //                                         c->bind(
    //                                             [this]()
    //                                             {
    //                                                 return texturePreviewRequest.gamma;
    //                                             },
    //                                             [this](float v)
    //                                             {
    //                                                 texturePreviewRequest.gamma = v;
    //                                                 // 3. Здесь можно вызвать принудительную
    //                                                 // перерисовку окна превью
    //                                                 // modalTexturePreview->update();
    //                                             }
    //                                         );
    //                                     }
    //                                 )
    //                                 .relativeWidth(0.6f)
    //                                 .relativeHeight(1.0f)
    //                         )
    //                 )
    //                 // Filtering
    //                 .child(
    //                     LayoutBuilder::hBox()
    //                         .relativeWidth(1.0f)
    //                         .absoluteHeight(30)
    //                         .child(
    //                             LayoutBuilder::label(L"Filtering")
    //                                 .relativeWidth(0.4f)
    //                                 .relativeHeight(1.0f)
    //                         )
    //                         .child(
    //                             LayoutBuilder::widget(new Widgets::ComboBox())
    //                                 .relativeWidth(0.6f)
    //                                 .relativeHeight(1.0f)
    //                                 .apply<Widgets::ComboBox>(
    //                                     [&](Widgets::ComboBox* c)
    //                                     {
    //                                         c->addItem({L"Point", 0});
    //                                         c->addItem({L"Bilinear", 1});
    //                                         c->addItem({L"Trilinear", 2});
    //                                     }
    //                                 )
    //                         )
    //                 )
    //         )

    //         // Пружина (занимает всё оставшееся свободное место, выталкивая кнопки вниз)
    //         .child(LayoutBuilder::spacer().relativeWidth(1.0f).relativeHeight(1.0f))

    //         // --- СЕКЦИЯ: КНОПКИ ДЕЙСТВИЙ ---
    //         .child(
    //             LayoutBuilder::hBox()
    //                 .relativeWidth(1.0f)
    //                 .absoluteHeight(40)
    //                 .child(
    //                     LayoutBuilder::widget(new Widgets::Button())
    //                         .text(L"REIMPORT")
    //                         .relativeWidth(0.5f)
    //                         .relativeHeight(1.0f)
    //                 )
    //                 .child(
    //                     LayoutBuilder::widget(new Widgets::Button())
    //                         .text(L"EXPORT...")
    //                         .relativeWidth(0.5f)
    //                         .relativeHeight(1.0f)
    //                 )
    //         )
    //         .build();


    // modalInspector->getLayoutRoot()->addChild(std::move(inspectorUi));
    // modalInspector->show();
}

void EditorApp::rebuildInspector() noexcept
{
    using namespace nbui;

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
        }
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

    if (typeName.find("Vector3") != std::string::npos)
    {
        auto row = LayoutBuilder::hBox().relativeWidth(1.0f).absoluteHeight(30);

        row = std::move(row).child(
            LayoutBuilder::label(nb::Utils::toWString(field.name))
                .relativeWidth(0.35f)
                .color({180, 180, 180})
                .textAlignment({.textAlignment = TextAlignment::LEFT})

        );

        auto fieldsBox = LayoutBuilder::hBox().relativeWidth(0.65f);
        NbColor colors[] = {{255, 100, 100}, {100, 255, 100}, {100, 100, 255}};

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


    return parentBuilder;
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
        if (std::string(field.name) == "dirty")
        {
            void* bytePtr = static_cast<char*>(componentPtr) + field.offset;
            bool* dirtyPtr = reinterpret_cast<bool*>(bytePtr);

            *dirtyPtr = true;
            return;
        }
    }
}
