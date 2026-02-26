// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "App.hpp"


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

#include <Widgets/Section.hpp>


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

    settingsWindow = std::make_shared<Win32Window::ChildWindow>(mainWindow.get());
    settingsWindow->setTitle(
        Utils::toWstring(Translation::fromKey("Ui.Editor.Settings.Title"))
    );

    textureInspector = std::make_shared<Win32Window::ChildWindow>(mainWindow.get(), true);
    textureInspector->setTitle(
        Utils::toWstring(Translation::fromKey("Ui.Editor.TextureView.Title"))
    );

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
        settingsWindow,
        Temp::DockPosition::BOTTOM, 
        sceneTab->getWindow(),
        Temp::Percent(40)
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
        textureInspector,
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

    engine->getRenderer()->createSharedContextForWindow(textureInspector->getHandle().as<HWND>());
    textureInspector->setRenderable(false);
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
            .apply<Widgets::TreeView>([&](auto* tv)
                { 
                    tv->setModel(sceneModel);
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

void EditorApp::setupSettingsUI() noexcept
{
    using namespace nbui;

    auto ui = LayoutBuilder::hBox()
        .style([](NNsLayout::LayoutStyle& s) {
        s.padding = { 0, 0, 0, 0 };
        s.color = NbColor{ 30, 30, 30 };
        s.heightSizeType = NNsLayout::SizeType::RELATIVE;
        s.height = 1.0f;
            })

        .child(
            LayoutBuilder::vBox()
            .style([](NNsLayout::LayoutStyle& s) {
                s.widthSizeType = NNsLayout::SizeType::RELATIVE;
                s.width = 0.25f;
                s.heightSizeType = NNsLayout::SizeType::RELATIVE;
                s.height = 1.0f;
                s.padding = { 0, 0, 0, 0 };
                s.color = NbColor{ 40, 40, 40 };
                })

            .child(LayoutBuilder::label(L"SETTINGS")
                .relativeWidth(1.0f)
                .absoluteHeight(50)
                .margin({ 0, 0, 0, 0 })
                .background(NbColor{ 60, 60, 60 })
                .color(NbColor{ 220, 220, 220 })
                .fontSize(16)
                .textAlign(Widgets::TextAlign::CENTER))

            .child(LayoutBuilder::widget(new Widgets::Button())
                .text(L"PROJECT SETTINGS")
                .relativeWidth(1.0f)
                .absoluteHeight(40)
                .margin({ 0, 0, 0, 0 })
                .background(NbColor{ 50, 50, 50 })
                .color(NbColor{ 200, 200, 200 })
                .textAlign(Widgets::TextAlign::LEFT)
                .padding({ 0, 0, 0, 0 }))

            .child(LayoutBuilder::widget(new Widgets::Button())
                .text(L"EDITOR PREFERENCES")
                .relativeWidth(1.0f)
                .absoluteHeight(40)
                .margin({ 0, 0, 0, 0 })
                .background(NbColor{ 70, 70, 70 })
                .color(NbColor{ 255, 255, 255 })
                .textAlign(Widgets::TextAlign::LEFT)
                .padding({ 0, 0, 0, 0 }))

            .child(LayoutBuilder::widget(new Widgets::Button())
                .text(L"PLUGINS")
                .relativeWidth(1.0f)
                .absoluteHeight(40)
                .margin({ 0, 0, 0, 0 })
                .background(NbColor{ 50, 50, 50 })
                .color(NbColor{ 200, 200, 200 })
                .textAlign(Widgets::TextAlign::LEFT)
                .padding({ 0, 0, 0, 0 }))

            .child(LayoutBuilder::widget(new Widgets::Button())
                .text(L"KEYBOARD SHORTCUTS")
                .relativeWidth(1.0f)
                .absoluteHeight(40)
                .margin({ 0, 0, 0, 0 })
                .background(NbColor{ 50, 50, 50 })
                .color(NbColor{ 200, 200, 200 })
                .textAlign(Widgets::TextAlign::LEFT)
                .padding({ 0, 0, 0, 0 }))

            .child(LayoutBuilder::widget(new Widgets::Button())
                .text(L"APPEARANCE")
                .relativeWidth(1.0f)
                .absoluteHeight(40)
                .margin({ 0, 0, 0, 0 })
                .background(NbColor{ 50, 50, 50 })
                .color(NbColor{ 200, 200, 200 })
                .textAlign(Widgets::TextAlign::LEFT)
                .padding({ 0, 0, 0, 0 }))

            .child(LayoutBuilder::widget(new Widgets::Button())
                .text(L"PERFORMANCE")
                .relativeWidth(1.0f)
                .absoluteHeight(40)
                .margin({ 0, 0, 0, 0 })
                .background(NbColor{ 50, 50, 50 })
                .color(NbColor{ 200, 200, 200 })
                .textAlign(Widgets::TextAlign::LEFT)
                .padding({ 0, 0, 0, 0 }))

            .child(LayoutBuilder::widget(new Widgets::Button())
                .text(L"SOURCE CODE")
                .relativeWidth(1.0f)
                .absoluteHeight(40)
                .margin({ 0, 0, 0, 0 })
                .background(NbColor{ 50, 50, 50 })
                .color(NbColor{ 200, 200, 200 })
                .textAlign(Widgets::TextAlign::LEFT)
                .padding({ 0, 0, 0, 0 }))

            .child(LayoutBuilder::spacer())

            .child(LayoutBuilder::widget(new Widgets::Button())
                .text(L"SAVE")
                .relativeWidth(1.0f)
                .absoluteHeight(40)
                .margin({ 0, 0, 0, 0 })
                .background(NbColor{ 0, 120, 0 })
                .color(NbColor{ 255, 255, 255 })
                .textAlign(Widgets::TextAlign::CENTER))

            .child(LayoutBuilder::widget(new Widgets::Button())
                .text(L"CANCEL")
                .relativeWidth(1.0f)
                .absoluteHeight(40)
                .margin({ 0, 0, 0, 0 })
                .background(NbColor{ 80, 80, 80 })
                .color(NbColor{ 220, 220, 220 })
                .textAlign(Widgets::TextAlign::CENTER))

            .child(LayoutBuilder::widget(new Widgets::Button())
                .text(L"RESET")
                .relativeWidth(1.0f)
                .absoluteHeight(40)
                .margin({ 0, 0, 0, 0 })
                .background(NbColor{ 120, 0, 0 })
                .color(NbColor{ 255, 255, 255 })
                .textAlign(Widgets::TextAlign::CENTER))
        )
        .child(
            LayoutBuilder::vBox()
            .style([](NNsLayout::LayoutStyle& s) {
                s.widthSizeType = NNsLayout::SizeType::RELATIVE;
                s.width = 0.75f;
                s.heightSizeType = NNsLayout::SizeType::RELATIVE;
                s.height = 1.0f;
                s.padding = { 15, 15, 15, 15 };
                s.color = NbColor{ 50, 50, 50 };
                })

            .child(LayoutBuilder::label(L"EDITOR PREFERENCES")
                .relativeWidth(1.0f)
                .absoluteHeight(50)
                .margin({ 0, 0, 0, 0 })
                .background(NbColor{ 60, 60, 60 })
                .color(NbColor{ 255, 255, 255 })
                .fontSize(20)
                .textAlign(Widgets::TextAlign::LEFT)
                .padding({ 0, 0, 0, 0 }))

            .child(
                LayoutBuilder::hBox()
                .style([](NNsLayout::LayoutStyle& s) {
                    s.heightSizeType = NNsLayout::SizeType::ABSOLUTE;
                    s.height = 35;
                    s.margin = { 0, 0, 15, 0 };
                    })
                .child(LayoutBuilder::widget(new Widgets::Button())
                    .text(L"GENERAL")
                    .relativeWidth(0.2f)
                    .absoluteHeight(35)
                    .margin({ 0, 5, 0, 0 })
                    .background(NbColor{ 80, 80, 80 })
                    .color(NbColor{ 255, 255, 255 }))

                .child(LayoutBuilder::widget(new Widgets::Button())
                    .text(L"REGIONAL")
                    .relativeWidth(0.2f)
                    .absoluteHeight(35)
                    .margin({ 0, 5, 0, 0 })
                    .background(NbColor{ 70, 70, 70 })
                    .color(NbColor{ 200, 200, 200 }))

                .child(LayoutBuilder::widget(new Widgets::Button())
                    .text(L"SOURCE CODE")
                    .relativeWidth(0.2f)
                    .absoluteHeight(35)
                    .margin({ 0, 5, 0, 0 })
                    .background(NbColor{ 70, 70, 70 })
                    .color(NbColor{ 200, 200, 200 }))

                .child(LayoutBuilder::widget(new Widgets::Button())
                    .text(L"DEBUGGING")
                    .relativeWidth(0.2f)
                    .absoluteHeight(35)
                    .margin({ 0, 5, 0, 0 })
                    .background(NbColor{ 70, 70, 70 })
                    .color(NbColor{ 200, 200, 200 }))

                .child(LayoutBuilder::widget(new Widgets::Button())
                    .text(L"EXPERIMENTAL")
                    .relativeWidth(0.2f)
                    .absoluteHeight(35)
                    .margin({ 0, 0, 0, 0 })
                    .background(NbColor{ 70, 70, 70 })
                    .color(NbColor{ 200, 200, 200 }))
            )

            .child(
                LayoutBuilder::vBox()
                .style([](NNsLayout::LayoutStyle& s) {
                    s.heightSizeType = NNsLayout::SizeType::RELATIVE;
                    s.height = 1.0f;
                    s.padding = { 0, 0, 0, 0 };
                    })

                .child(
                    LayoutBuilder::vBox()
                    .style([](NNsLayout::LayoutStyle& s) {
                        s.margin = { 0, 0, 15, 0 };
                        s.padding = { 10, 10, 10, 10 };
                        s.color = NbColor{ 45, 45, 45 };
                        s.border.width = 1;
                        })

                    .child(LayoutBuilder::label(L"Auto Save")
                        .relativeWidth(1.0f)
                        .absoluteHeight(35)
                        .margin({ 0, 0, 8, 0 })
                        .background(NbColor{ 60, 60, 60 })
                        .color(NbColor{ 220, 220, 220 })
                        .fontSize(16)
                        .textAlign(Widgets::TextAlign::LEFT))

                    .child(LayoutBuilder::widget(new Widgets::CheckBox())
                        .text(L"Enable Auto Save")
                        .relativeWidth(1.0f)
                        .absoluteHeight(35)
                        .margin({ 0, 0, 8, 0 })
                        .background(NbColor{ 80, 80, 80 })
                        .color(NbColor{ 200, 200, 200 })
                        .textAlign(Widgets::TextAlign::LEFT)
                        .padding({ 0, 0, 0, 0 })
                        .onEvent(&Widgets::CheckBox::onCheckStateChanged,
                            [&](bool checked) {
                                Debug::debug(L"Auto Save: " + std::wstring(checked ? L"Enabled" : L"Disabled"));
                            }))

                    .child(
                        LayoutBuilder::hBox()
                        .style([](NNsLayout::LayoutStyle& s) {
                            s.heightSizeType = NNsLayout::SizeType::ABSOLUTE;
                            s.height = 35;
                            s.margin = { 0, 0, 8, 0 };
                            })

                        .child(LayoutBuilder::widget(new Widgets::IntSpinBox())
                            .relativeWidth(0.7f)
                            .absoluteHeight(35)
                            .color(NbColor{ 200, 200, 200 })
                            .textAlign(Widgets::TextAlign::LEFT)
                            .padding({ 0, 0, 0, 0 })
                        )

                        .child(LayoutBuilder::widget(new Widgets::IntSpinBox())
                            .relativeWidth(0.2f)
                            .absoluteHeight(35)
                            .background(NbColor{ 80, 80, 80 })
                            .color(NbColor{ 220, 220, 220 })
                            .textAlign(Widgets::TextAlign::CENTER)
                        )
                    )

                    .child(LayoutBuilder::widget(new Widgets::CalendarWidget({}, new Localization::HebrewCalendar))
                        .text(L"Save Only Current File")
                        .relativeWidth(1.0f)
                        .absoluteHeight(400)
                        .margin({ 0, 0, 0, 0 })
                        .background(NbColor{ 80, 80, 80 })
                        .color(NbColor{ 200, 200, 200 })
                        .textAlign(Widgets::TextAlign::LEFT)
                        .padding({ 0, 0, 0, 0 }))
                )

            )
        ).build();

    settingsWindow->getLayoutRoot()->addChild(std::move(ui));
}

void EditorApp::setupEngineDependentUi() noexcept
{
    setupDebugUI();
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
            .text(L"Show vertex color")
            .relativeWidth(1.0f).absoluteHeight(30)
            .onEvent(&Widgets::CheckBox::onCheckStateChanged, [&](bool checked) {
                engine->getRenderer()->showVertexColor(checked);
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
            .onEvent(&Widgets::CheckBox::onCheckStateChanged, [](bool checked) {
                //g_engine->getRenderer()->setDrawAABB(checked);
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
                        .textAlign(Widgets::TextAlign::LEFT)
                );

            for (const auto& field : info->fields)
            {
                inspectorBuilder = buildFieldUI(
                    std::move(inspectorBuilder),
                    data,
                    info,
                    field
                );
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
                .textAlign(Widgets::TextAlign::LEFT)
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
                                    .textAlign(Widgets::TextAlign::LEFT)
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
