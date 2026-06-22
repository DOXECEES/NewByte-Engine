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
#include <Widgets/ShaderCanvas.hpp>
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


static Ref<nb::Renderer::Mesh> createPrimitiveMesh(std::string_view typeName, const void* data) noexcept
{
    if (typeName == "CubeParams") {
        return nb::Renderer::PrimitiveGenerators::createCube(static_cast<const CubeParams*>(data)->size);
    }
    if (typeName == "SphereParams") {
        const auto* p = static_cast<const SphereParams*>(data);
        return nb::Renderer::PrimitiveGenerators::createSphere(p->radius, p->xSegments, p->ySegments);
    }
    if (typeName == "TorusParams") {
        const auto* p = static_cast<const TorusParams*>(data);
        return nb::Renderer::PrimitiveGenerators::createTorus(
            {static_cast<uint32>(p->xSegments), static_cast<uint32>(p->ySegments)}, 
            p->majorRadius, p->minorRadius
        );
    }
    if (typeName == "CylinderParams") {
        const auto* p = static_cast<const CylinderParams*>(data);
        return nb::Renderer::PrimitiveGenerators::createCylinder(p->radius, p->height, p->xSegments, p->ySegments);
    }
    if (typeName == "PlaneParams") {
        const auto* p = static_cast<const PlaneParams*>(data);
        return nb::Renderer::PrimitiveGenerators::createPlane(p->width, p->height, p->xSegments, p->ySegments);
    }
    if (typeName == "ConeParams") {
        const auto* p = static_cast<const ConeParams*>(data);
        return nb::Renderer::PrimitiveGenerators::createCone(p->radius, p->height, p->radialSegments, p->heightSegments);
    }
    if (typeName == "PyramidParams") {
        const auto* p = static_cast<const PyramidParams*>(data);
        return nb::Renderer::PrimitiveGenerators::createPyramid(p->radius, p->height, p->sides);
    }
    return nullptr;
}


static std::shared_ptr<Win32Window::ModalWindow> recreateModalWindow(
    std::shared_ptr<Win32Window::ModalWindow>& windowTracker,
    WindowInterface::IWindow* parent,
    NbSize<int> size,
    const std::wstring& title = L""
) {
    if (windowTracker)
    {
        windowTracker = nullptr;
    }

    auto newWin = std::make_shared<Win32Window::ModalWindow>(size, parent);
    windowTracker = newWin;

    if (!title.empty())
    {
        newWin->setTitle(title);
    }

    return newWin;
}

void EditorApp::requestModelSpawn(const SpawnModelParams& params) noexcept
{
    spawnQueue.pushBack(params);
}

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


    // sceneTabWindow = std::make_shared<Win32Window::ChildWindow>(mainWindow.get(), true);
    // sceneTabWindow->setTitle(
    //     Utils::toWstring(Translation::fromKey("Ui.Editor.SceneTab.Title"))
    // );

    // sceneToolbar = std::make_shared<Win32Window::ChildWindow>(sceneTabWindow.get());
    // sceneToolbar->setTitle(
    //     Utils::toWstring(Translation::fromKey("Ui.Editor.SceneToolbar.Title"))
    // );

    // sceneWindow = std::make_shared<Win32Window::ChildWindow>(sceneTabWindow.get(), true);
    // sceneWindow->setTitle(
    //     Utils::toWstring(Translation::fromKey("Ui.Editor.Scene.Title"))
    // );

    sceneWindow = std::make_shared<sdk::SceneWindow>(mainWindow.get());
    sceneWindow->initialize();

    hierarchyWindow = std::make_shared<Win32Window::ChildWindow>(mainWindow.get());
    hierarchyWindow->setTitle(
        Utils::toWstring(Translation::fromKey("Ui.Editor.Hierarchy.Title"))
    );

    inspectorWindow = std::make_shared<Win32Window::ChildWindow>(mainWindow.get());
    inspectorWindow->setTitle(
        Utils::toWstring(Translation::fromKey("Ui.Editor.Inspector.Title"))
    );

    assetManager = std::make_shared<Win32Window::ChildWindow>(mainWindow.get());
    assetManager->setTitle(
        Localization::Translation::fromKeyToWstring("Ui.Editor.AssetManager.Title")
    );
    
    debugWindow = std::make_shared<Win32Window::ChildWindow>(mainWindow.get());
    debugWindow->setTitle(
        Utils::toWstring(Translation::fromKey("Ui.Editor.DebugWindow.Title"))
    );

    toolbarWindow = std::make_shared<Win32Window::ChildWindow>(mainWindow.get());
    toolbarWindow->setTitle(L"Toolbar");

    previewWindow = std::make_shared<Win32Window::ChildWindow>(nullptr);
    previewWindow->setTitle(L"prev");
    previewWindow->addCaption();
    previewWindow->setRenderable(false);

    shaderNodes = std::make_shared<Win32Window::ChildWindow>(nullptr);
    shaderNodes->setTitle(L"Shader");
    shaderNodes->addCaption();

    auto shaderNodesUi = nbui::LayoutBuilder::vBox()
                             .relativeHeight(1.0f)
                             .relativeWidth(1.0f)
                             .child(
                                 nbui::LayoutBuilder::widget(new Widgets::ShaderCanvas())
                                     .relativeHeight(1.0f)
                                     .relativeWidth(1.0f)
                             )
                             .build();

    shaderNodes->getLayoutRoot()->addChild(std::move(shaderNodesUi));
}

void EditorApp::setupDocking() noexcept
{
    dockManager = std::make_unique<Temp::DockingSystem>(mainWindow);
    mainWindow->setDockingSystem(dockManager.get());

    // 1. Scene as the base
    auto sceneTab = dockManager->dockAsTab(sceneWindow->getTabWindow(), nullptr, "Scene");

    // 2. Hierarchy to the left of scene
    dockManager->dockRelative(
        hierarchyWindow, Temp::DockPosition::LEFT, sceneWindow->getTabWindow(), Temp::Percent(20)
    );

    // 3. Inspector to the right
    dockManager->dockRelative(
        inspectorWindow, Temp::DockPosition::RIGHT, nullptr, Temp::Percent(25)
    );

    // 4. Add tabs INTO the inspector group — do ALL of them before any further dockRelative calls
    dockManager->dockAsTab(debugWindow, inspectorWindow, "debug");

    // 5. Asset manager at the bottom — AFTER the inspector group is fully built
    dockManager->dockRelative(
        assetManager, Temp::DockPosition::BOTTOM, nullptr, Temp::Percent(40)
    );

    // auto toolbarTab = std::dynamic_pointer_cast<Temp::TabNode>(
    //     dockManager->dockRelative(toolbarWindow, Temp::DockPosition::TOP, nullptr, Temp::Percent(3))
    // );

    // if (toolbarTab)
    // {
    //     if (auto group = toolbarTab->getTabGroup())
    //         group->setShowTabBar(false);
    // }

   
    subscribe(
        *mainWindow, &Win32Window::Window::onRectChanged,
        [this](const NbRect<int>& rect)
        {
            toolbarWindow->setSize({rect.width, 35});
            toolbarWindow->setPosition({rect.x, rect.y});
            dockManager->onSize(rect.width, rect.height);
            OutputDebugStringW(dockManager->dumpTreeW().c_str());

            sceneWindow->handleResize(rect);
        }
    );


}

void EditorApp::initEngine() noexcept
{
    engine = std::make_shared<nb::Core::Engine>(sceneWindow->getViewportWindow()->getHandle().as<HWND>());
    auto& scene = nb::Scene::getInstance();

    sceneModel = std::make_shared<SceneModelEcs>(scene.getRegistry(), scene.getRootEntity().id);
    
    const auto& size = sceneWindow->getViewportWindow()->getSize();
    nb::Core::EngineSettings::setHeight(size.height);
    nb::Core::EngineSettings::setWidth(size.width);

    subscribe(*(sceneWindow->getViewportWindow()), &Win32Window::ChildWindow::onSizeChanged, [](const NbSize<int>& s) {
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
                LayoutBuilder::hBox()
                    .absoluteHeight(35.0f)
                    .relativeWidth(1.0f)
                    .style(
                        [](NNsLayout::LayoutStyle& s)
                        {
                            s.color = NbColor{45, 45, 45};
                        }
                    )

                    .child(
                        createMenuButton(
                            "Ui.Editor.File",
                            [this](PopupMenu* popup)
                            {
                                popup->addItem(
                                    Localization::Translation::fromKeyToWstring(
                                        "Ui.Editor.NewProject"
                                    ),
                                    IconType::Plus,
                                    [this]()
                                    {
                                        openFilePicker(
                                            Localization::Translation::fromKeyToWstring(
                                                "Ui.Editor.CreateNewProject"
                                            ),
                                            [this](const std::string& selectedPath)
                                            {
                                                namespace fs = std::filesystem;
                                                fs::path p(selectedPath);

                                                if (p.extension() != ".json")
                                                {
                                                    p += ".json";
                                                }

                                                try
                                                {
                                                    if (!fs::exists(p.parent_path()))
                                                    {
                                                        fs::create_directories(p.parent_path());
                                                    }

                                                    engine->saveSnapshot();
                                                    engine->clearScene();
                                                    sceneModel->rebuildFromScene();
                                                    sceneController->setActiveNode(nb::Node::createInvalid());

                                                    refreshHierarchyTreeViewSignal.emit();
                                                    onActiveNodeChanged.emit();
                                                    nb::Scene::getInstance().invalidateBvh();

                                                    engine->setProjectPath(p.string());
                                                    engine->saveSnapshot();

                                                    nb::Error::ErrorManager::instance().report(
                                                        nb::Error::Type::INFO,
                                                        "New project created at: " + p.string()
                                                    );

                                                    shouldRebuildInspector = true;

                                                    engine->loadSnapshot();
                                                }
                                                catch (const std::exception& e)
                                                {
                                                    nb::Error::ErrorManager::instance().report(
                                                        nb::Error::Type::FATAL,
                                                        std::string(
                                                            "Failed to create "
                                                            "project: "
                                                        ) + e.what()
                                                    );
                                                }
                                            },
                                            toolbarWindow.get(), {".json"}
                                        );
                                    }
                                );
                                popup->addItem(
                                    Localization::Translation::fromKeyToWstring("Ui.Editor.Open"),
                                    IconType::None,
                                    [this]()
                                    {
                                        openFilePicker(
                                            Localization::Translation::fromKeyToWstring(
                                                "Ui.Editor.SelectResource"
                                            ),
                                            [this](const std::string& path)
                                            {
                                                nb::Error::ErrorManager::instance().report(
                                                    nb::Error::Type::INFO, path
                                                );

                                                engine->saveSnapshot();
                                                engine->clearScene();

                                                engine->setProjectPath(path);
                                                engine->loadSnapshot();

                                                sceneModel->rebuildFromScene();
                                                sceneController->setActiveNode(nb::Node::createInvalid());

                                                refreshHierarchyTreeViewSignal.emit();
                                                onActiveNodeChanged.emit();
                                                nb::Scene::getInstance().invalidateBvh();
                                            },
                                            toolbarWindow.get(), {".json"}
                                        );
                                    }
                                );
                                popup->addSeparator();
                                popup->addItem(
                                    Localization::Translation::fromKeyToWstring("Ui.Editor.Save"),
                                    IconType::None,
                                    [this]()
                                    {
                                        engine->saveSnapshot();
                                    }
                                );
                                popup->addSeparator();
                                popup->addItem(
                                    Localization::Translation::fromKeyToWstring("Ui.Editor.Exit"),
                                    IconType::Delete,
                                    [this]()
                                    {
                                        PostQuitMessage(0);
                                    }
                                );
                            }
                        )

                    )
                    .child(createMenuButton(
                        "Ui.Editor.Edit",
                        [this](PopupMenu* popup)
                        {
                            popup->addItem(
                                L"New Project", IconType::Plus,
                                [this]()
                                {
                                }
                            );
                            popup->addItem(
                                L"Open...", IconType::None,
                                [this]()
                                {
                                    openFilePicker(
                                        L"Select Resource: ",
                                        [this](const std::string& path)
                                        {
                                            nb::Error::ErrorManager::instance().report(
                                                nb::Error::Type::INFO, path
                                            );
                                        },
                                        toolbarWindow.get()
                                    );

                                    // engine->loadSnapshot()
                                }
                            );
                            popup->addSeparator();
                            popup->addItem(
                                L"Save", IconType::None,
                                [this]()
                                {
                                    engine->saveSnapshot();
                                }
                            );
                            popup->addSeparator();
                            popup->addItem(
                                L"Exit", IconType::Delete,
                                [this]()
                                {
                                    PostQuitMessage(0);
                                }
                            );
                        }
                    ))

                    .child(createMenuButton(
                        "Ui.Editor.View",
                        [this](PopupMenu* popup)
                        {
                            popup->addItem(
                                Localization::Translation::fromKeyToWstring("Ui.Editor.View.Lang"),
                                IconType::Plus,
                                [this]()
                                {
                                    if (languagePicker)
                                    {
                                        languagePicker = nullptr;
                                    }

                                    languagePicker = std::make_shared<Win32Window::ModalWindow>(
                                        NbSize<int>{400, 120}, toolbarWindow.get()
                                    );
                                    languagePicker->setTitle(
                                        Localization::Translation::fromKeyToWstring(
                                            "Ui.Editor.Language"
                                        )
                                    );

                                    struct State
                                    {
                                        int selectedLang = 0;
                                    };
                                    auto state = std::make_shared<State>();

                                    auto ui =
                                        LayoutBuilder::vBox()
                                            .spacing(0)
                                            .relativeWidth(1.0f)
                                            .relativeHeight(1.0f)
                                            .background({35, 35, 35})
                                            .child(
                                                LayoutBuilder::hBox()
                                                    .relativeWidth(1.0f)
                                                    .absoluteHeight(35)
                                                    .child(
                                                        LayoutBuilder::label(
                                                            Localization::Translation::
                                                                fromKeyToWstring(
                                                                    "Ui.Editor."
                                                                    "Language"
                                                                )
                                                        )
                                                            .relativeWidth(0.4f)
                                                            .color({200, 200, 200})
                                                            .textAlignment(
                                                                {.textAlignment =
                                                                     TextAlignment::LEFT}
                                                            )
                                                    )
                                                    .child(
                                                        LayoutBuilder::widget(
                                                            new Widgets::ComboBox()
                                                        )
                                                            .relativeWidth(0.6f)
                                                            .absoluteHeight(30)
                                                            .apply<Widgets::ComboBox>(
                                                                [state](Widgets::ComboBox* cb)
                                                                {
                                                                    cb->addItem({L"Русский", 0});
                                                                    cb->addItem({L"English", 1});

                                                                    cb->setSelectedItem(0);
                                                                }
                                                            )
                                                            .onEvent(
                                                                &Widgets::ComboBox::
                                                                    onSelectionChanged,
                                                                [state](
                                                                    const Widgets::ListItem& item
                                                                )
                                                                {
                                                                    state->selectedLang =
                                                                        item.getValue<int>();
                                                                }
                                                            )
                                                    )
                                            )
                                            .child(LayoutBuilder::spacer())
                                            .child(
                                                LayoutBuilder::hBox()
                                                    .relativeWidth(1.0f)
                                                    .absoluteHeight(35)
                                                    .spacing(10)
                                                    .child(
                                                        LayoutBuilder::spacer().relativeWidth(0.2f)
                                                    )
                                                    .child(
                                                        LayoutBuilder::widget(new Widgets::Button())
                                                            .text(
                                                                Localization::Translation::
                                                                    fromKeyToWstring(
                                                                        "Ui."
                                                                        "Editor"
                                                                        ".Save"
                                                                    )
                                                            )
                                                            .relativeWidth(0.4f)
                                                            .background({60, 60, 60})
                                                            .onEvent(
                                                                &Widgets::Button::onReleasedSignal,
                                                                [this, state]()
                                                                {
                                                                    if (state->selectedLang == 0)
                                                                    {
                                                                        Localization::Translation::
                                                                            load(
                                                                                "Assets/"
                                                                                "Localization/"
                                                                                "ru.translation"
                                                                            );
                                                                    }
                                                                    else
                                                                    {
                                                                        Localization::Translation::
                                                                            load(
                                                                                "Assets/"
                                                                                "Localization/"
                                                                                "en.translation"
                                                                            );
                                                                    }

                                                                    this->refreshInterfaceText();
                                                                    languagePicker->close();
                                                                }
                                                            )
                                                    )
                                                    .child(
                                                        LayoutBuilder::widget(new Widgets::Button())
                                                            .text(
                                                                Localization::Translation::
                                                                    fromKeyToWstring(
                                                                        "Ui."
                                                                        "Materi"
                                                                        "alEdit"
                                                                        "or."
                                                                        "Exit"
                                                                    )
                                                            )
                                                            .relativeWidth(0.4f)
                                                            .background({50, 50, 50})
                                                            .onEvent(
                                                                &Widgets::Button::onReleasedSignal,
                                                                [this]()
                                                                {
                                                                    languagePicker->close();
                                                                    languagePicker = nullptr;
                                                                }
                                                            )
                                                    )
                                            );

                                    languagePicker->getLayoutRoot()->addChild(
                                        std::move(ui).build()
                                    );
                                    languagePicker->show();
                                }
                            );

                            popup->addItem(
                                Localization::Translation::fromKeyToWstring(
                                    "Ui.Editor.FrameBufferVisualization"
                                ),
                                IconType::Edit,
                                [this]()
                                {
                                    // recreateModalWindow(
                                    //     framebufferVisualizationWindow, mainWindow.get(),
                                    //     NbSize<int>(800, 600),
                                    //     Localization::Translation::fromKeyToWstring(
                                    //         "Ui.Editor.FrameBufferVisualization"
                                    //     )
                                    // );

                                    if (framebufferVisualization)
                                    {
                                        framebufferVisualization = nullptr;
                                    }

                                    framebufferVisualization = std::make_shared<Sdk::FramebufferVisualization>(
                                        nullptr,
                                        engine.get()
                                        //nbstl::NonOwningPtr(materialRef.get())
                                    );

                                    //framebufferVisualization->show();

                                    // previewWindow =
                                    // std::make_shared<Win32Window::ChildWindow>(modalWindow.get(),
                                    // true);
                                    // auto sharedContext =
                                    // engine->getRenderer()->createSharedContextForWindow(framebufferVisualizationWindow->getHandle().as<HWND>());

                                    //
                                }
                            );

                            popup->addItem(
                                Localization::Translation::fromKeyToWstring(
                                    "Ui.Editor.GizmoToggle"
                                ),
                                IconType::Edit,
                                [this]()
                                {
                                    if (gizmoToggleWindow)
                                    {
                                        gizmoToggleWindow = nullptr;
                                    }

                                    // Создаем модальное окно размером 350x180 под управлением toolbarWindow
                                    gizmoToggleWindow = std::make_shared<Win32Window::ModalWindow>(
                                        NbSize<int>{350, 180}, toolbarWindow.get()
                                    );
                                    gizmoToggleWindow->setTitle(
                                        Localization::Translation::fromKeyToWstring(
                                            "Ui.Editor.GizmoToggle"
                                        )
                                    );

                                    auto ui =
                                        LayoutBuilder::vBox()
                                            //.spacing(10)
                                            //.padding(Padding<int>{15, 15, 15, 15})
                                            .relativeWidth(1.0f)
                                            .relativeHeight(1.0f)
                                            .background({35, 35, 35})
                                            .child(
                                                LayoutBuilder::widget(new Widgets::CheckBox())
                                                    .relativeWidth(1.0f)
                                                    .autoHeight()
                                                    .text(L"Показывать гизмо (Show Gizmo)")
                                                    .checked(debugRendererSettings.showGizmo) // Состояние по умолчанию
                                                    .onEvent(
                                                        &Widgets::CheckBox::onCheckStateChanged,
                                                        [this](bool state)
                                                        {
                                                            debugRendererSettings.showGizmo = state;
                                                            engineSettingsController->setDebugRendererSettings(debugRendererSettings); 
                                                        }
                                                    )
                                            )
                                            .child(
                                                LayoutBuilder::widget(new Widgets::CheckBox())
                                                    .relativeWidth(1.0f)
                                                    .autoHeight()
                                                    .text(L"Показывать фруструм камеры")
                                                    .checked(debugRendererSettings.showCameraFrustrum)
                                                    .onEvent(
                                                        &Widgets::CheckBox::onCheckStateChanged,
                                                        [this](bool state)
                                                        {
                                                            debugRendererSettings.showCameraFrustrum = state;
                                                            engineSettingsController->setDebugRendererSettings(debugRendererSettings); 
                                                        }
                                                    )
                                            )
                                            .child(
                                                LayoutBuilder::widget(new Widgets::CheckBox())
                                                    .relativeWidth(1.0f)
                                                    .autoHeight()
                                                    .text(L"Показывать отладочные билборды")
                                                    .checked(debugRendererSettings.showDebugBillboards)
                                                    .onEvent(
                                                        &Widgets::CheckBox::onCheckStateChanged,
                                                        [this](bool state)
                                                        {
                                                            debugRendererSettings.showDebugBillboards = state;
                                                            engineSettingsController->setDebugRendererSettings(debugRendererSettings); 
                                                        }
                                                    )
                                            )
                                            .child(
                                                LayoutBuilder::widget(new Widgets::CheckBox())
                                                    .relativeWidth(1.0f)
                                                    .autoHeight()
                                                    .text(L"Показывать световые гизмо")
                                                    .checked(debugRendererSettings.showLightGizmos)
                                                    .onEvent(
                                                        &Widgets::CheckBox::onCheckStateChanged,
                                                        [this](bool state)
                                                        {
                                                            debugRendererSettings.showLightGizmos = state;
                                                            engineSettingsController->setDebugRendererSettings(debugRendererSettings); 
                                                        }
                                                    )
                                            )
                                            .child(
                                                LayoutBuilder::widget(new Widgets::CheckBox())
                                                    .relativeWidth(1.0f)
                                                    .autoHeight()
                                                    .text(L"Показывать световые гизмо")
                                                    .checked(debugRendererSettings.showColliders)
                                                    .onEvent(
                                                        &Widgets::CheckBox::onCheckStateChanged,
                                                        [this](bool state)
                                                        {
                                                            debugRendererSettings.showColliders = state;
                                                            engineSettingsController->setDebugRendererSettings(debugRendererSettings); 
                                                        }
                                                    )
                                            )
                                            .child(LayoutBuilder::spacer())
                                            .child(
                                                LayoutBuilder::hBox()
                                                    .relativeWidth(1.0f)
                                                    .absoluteHeight(35)
                                                    .child(LayoutBuilder::spacer())
                                                    .child(
                                                        LayoutBuilder::widget(new Widgets::Button())
                                                            .text(
                                                                Localization::Translation::
                                                                    fromKeyToWstring(
                                                                        "Ui.MaterialEditor.Exit"
                                                                    )
                                                            )
                                                            .relativeWidth(0.4f)
                                                            .background({50, 50, 50})
                                                            .onEvent(
                                                                &Widgets::Button::onReleasedSignal,
                                                                [this]()
                                                                {
                                                                    gizmoToggleWindow->close();
                                                                    gizmoToggleWindow = nullptr;
                                                                }
                                                            )
                                                    )
                                            );

                                    gizmoToggleWindow->getLayoutRoot()->addChild(
                                        std::move(ui).build()
                                    );
                                    gizmoToggleWindow->show();
                                }
                            );

                        }
                    ))

                    .child(
                        LayoutBuilder::widget(new Widgets::Button())
                            .text(Localization::Translation::fromKeyToWstring("Ui.Editor.Help"))
                            .absoluteWidth(60.0f)
                            .relativeHeight(1.0f)
                    )
            )
            .child(
                LayoutBuilder::vBox()
                    .relativeWidth(1.0f)
                    .relativeHeight(1.0f)
                    .style(
                        [](NNsLayout::LayoutStyle& s)
                        {
                            s.margin = {5, 5, 5, 5};
                        }
                    )
                    .child(LayoutBuilder::widget(new Widgets::Label()).text(L"Main Editor Area"))
            )
            .build();

    toolbarWindow->getLayoutRoot()->addChild(std::move(rootUI));
}

void EditorApp::setupHierarchyUI() noexcept
{
    using namespace nbui;

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
        .child(
            LayoutBuilder::label(L"").absoluteWidth(110).absoluteHeight(80).background(
                NbColor{20, 20, 20}
            )
        )
        .child(LayoutBuilder::label(L"").absoluteWidth(110).absoluteHeight(5).background(typeColor))
        .child(
            LayoutBuilder::label(name)
                .absoluteWidth(110)
                .absoluteHeight(35)
                .fontSize(12)
                .color(NbColor{220, 220, 220})
        )
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
    cameraSettingsController.init(engine.get());
    setupDebugUI();
    setupAssetManager();
    debugWindow->show();

    cameraBookmarkWindow = std::make_unique<sdk::CameraBookmarkWindow>(mainWindow, cameraBookmarkManager, cameraSettingsController);
    engineSettingsController = std::make_unique<sdk::EngineSettingsController>(engine.get());
    
    sceneController = std::make_shared<sdk::SceneController>(engine, sceneModel, primitiveNameManager);
    subscribe(*sceneController, &sdk::SceneController::refreshHierarchySignal, [this]() { this->refreshHierarchyTreeViewSignal.emit(); });


    sceneWindow->attachController(sceneController);
    dockManager->dockAsTab(cameraBookmarkWindow->getWindow(), inspectorWindow, "Bookmarks");
    
    sharedContext = engine->getRenderer()->createSharedContextForWindow(previewWindow->getHandle().as<HWND>());
}

using namespace nbui;

// Строка со "слайдером": Label [150px] | Slider [Fill] | Value [40px]
using namespace nbui;

// Строка со "слайдером" (Label | Bar | Value)
LayoutBuilder makeSliderRow(
    const std::wstring& name,
    const std::wstring& value,
    float               progress
)
{
    return LayoutBuilder::hBox()
        .relativeWidth(1.0f)
        .absoluteHeight(26)
        //.spacing(10) // Промежутки между элементами в строке
        .child(
            LayoutBuilder::label(name).absoluteWidth(
                140
            ) //.textAlignment(TextFormatAlignment::RIGHT)
        )
        .child(
            LayoutBuilder::hBox() // Фон слайдера
                .relativeWidth(0.6f)
                .absoluteHeight(26)
                .background({45, 45, 45, 255})
                .border(1, Border::Style::SOLID, {70, 70, 70, 255})
                .child(
                    LayoutBuilder::widget(new Widgets::Slider<float>()) // Полоска прогресса
                        .relativeWidth(progress)
                        .relativeHeight(1.0f)
                )
        );
}

LayoutBuilder makeInputRow(
    const std::wstring& name,
    const std::wstring& value
)
{
    return LayoutBuilder::hBox()
        .relativeWidth(1.0f)
        .absoluteHeight(26)
        //.spacing(10)
        .child(
            LayoutBuilder::label(name).absoluteWidth(140)//.textAlignment(TextFormatAlignment::RIGHT)
        )
        .child(
            LayoutBuilder::widget(new Widgets::TextEdit())
                .absoluteWidth(60)
                .absoluteHeight(20)
                .background({25, 25, 25, 255})
                //.border(1, Border::Style::SOLID, {90, 90, 90, 255})
                //.textAlignment(TextFormatAlignment::CENTER)
        );
}

void EditorApp::setupDebugUI() noexcept
{
    using namespace nbui;

    auto ui =
        LayoutBuilder::vBox()
            .relativeWidth(1.0f)
            .relativeHeight(1.0f)
            .background({35, 35, 35, 255})
            //.padding({10, 10, 10, 10})
            .spacing(2)
            .child(
                LayoutBuilder::section(L"▼ Ambient Occlusion (SSAO)")
                    .relativeWidth(1.0f)
                    .autoHeight()
                    .style(
                        [this](auto& s)
                        {
                            s.color = {52, 52, 52};
                        }
                    )
                    .margin({10,10,0,10})
                    .child(
                        LayoutBuilder::vBox()
                            .relativeWidth(1.0f)
                            .autoHeight()
                            //.padding({10, 10, 10, 10}) 
                            .spacing(6)

                            .child(
                                LayoutBuilder::vBox()
                                    .relativeWidth(1.0f)
                                    .absoluteHeight(25)
                                    .child(
                                        LayoutBuilder::widget(new Widgets::CheckBox())
                                        .text(
                                            Localization::Translation::fromKeyToWstring(
                                                "Ui.Editor.SSAO.Enabled"
                                            )
                                        )
                                            .relativeWidth(1.0f)
                                            .relativeHeight(1.0f)
                                            .apply<Widgets::CheckBox>(
                                                [&](Widgets::CheckBox* c)
                                                {
                                                    c->setChecked(true);
                                                }
                                            )
                                            .onEvent(&Widgets::CheckBox::onCheckStateChanged, [&](bool checked) {
                                                engine->getRenderer()->toggleSsao();
                                            })
                                    )
                            )

                            .child(
                                LayoutBuilder::hBox()
                                    .relativeWidth(1.0f)
                                    .absoluteHeight(26)
                                    .child(
                                        LayoutBuilder::label(
                                            Localization::Translation::fromKeyToWstring(
                                                "Ui.Editor.SSAO.Radius"
                                            )
                                        )
                                            .absoluteWidth(
                                            140
                                        ) 
                                    )
                                    .child(
                                        LayoutBuilder::hBox()
                                            .relativeWidth(0.6f)
                                            .absoluteHeight(26)
                                            .background({45, 45, 45, 255})
                                            .border(1, Border::Style::SOLID, {70, 70, 70, 255})
                                            .child(
                                                LayoutBuilder::widget(
                                                    new Widgets::Slider<float>()
                                                ) 
                                                    .relativeWidth(1.0f)
                                                    .relativeHeight(1.0f)
                                                    .apply<Widgets::Slider<float>>([&](Widgets::Slider<float>* slid)
                                                        {
                                                            slid->bind(
                                                                [&]()
                                                                {
                                                                    return engine->getRenderer()
                                                                        ->getSSAOConfig()
                                                                        .radius;
                                                                },
                                                                [&](float val)
                                                                {
                                                                    engine->getRenderer()
                                                                               ->getSSAOConfig()
                                                                               .radius = val;
                                                                }
                                                            );
                                                        }
                                                    )
                                            )
                                    )
                            )
                            .child(
                                LayoutBuilder::hBox()
                                    .relativeWidth(1.0f)
                                    .absoluteHeight(26)
                                    .child(
                                        LayoutBuilder::label(
                                            Localization::Translation::fromKeyToWstring(
                                                "Ui.Editor.SSAO.Bias"
                                            )
                                        )
                                            .absoluteWidth(
                                                140
                                            ) 
                                    )
                                    .child(
                                        LayoutBuilder::hBox() 
                                            .relativeWidth(0.6f)
                                            .absoluteHeight(26)
                                            .background({45, 45, 45, 255})
                                            .border(1, Border::Style::SOLID, {70, 70, 70, 255})
                                            .child(
                                                LayoutBuilder::widget(
                                                    new Widgets::Slider<float>()
                                                ) 
                                                    .relativeWidth(1.0f)
                                                    .relativeHeight(1.0f)
                                                    .apply<Widgets::Slider<float>>(
                                                        [&](Widgets::Slider<float>* slid)
                                                        {
                                                            slid->bind(
                                                                [&]()
                                                                {
                                                                    return engine->getRenderer()
                                                                        ->getSSAOConfig()
                                                                        .bias;
                                                                },
                                                                [&](float val)
                                                                {
                                                                    engine->getRenderer()
                                                                        ->getSSAOConfig()
                                                                        .bias = val;
                                                                }
                                                            );
                                                        }
                                                    )
                                            )
                                    )
                            )

                    )
            )
            .child(
                LayoutBuilder::section(
                    Localization::Translation::fromKeyToWstring("Ui.Editor.ColorGrading")
                )
                    .relativeWidth(1.0f)
                    .autoHeight()
                    .style(
                        [this](auto& s)
                        {
                            s.color = {52, 52, 52};
                        }
                    )
                    .margin({0, 10, 0, 10})
                    .child(
                        LayoutBuilder::vBox()
                        .relativeWidth(1.0f)
                        .absoluteHeight(30.0f)
                        .child(
                            LayoutBuilder::widget(new Widgets::CheckBox())
                            .relativeHeight(1.0f)
                            .relativeWidth(1.0f)
                                        .text(
                                            Localization::Translation::fromKeyToWstring(
                                                "Ui.Editor.ColorGrading.Enabled"
                                            )
                                        )
                            .apply<Widgets::CheckBox>([](Widgets::CheckBox* c) { c->setChecked(true); })
                            .onEvent(&Widgets::CheckBox::onCheckStateChanged, [&](bool checked) {
                                engine->getRenderer()->getPostProcessConfig().isLutEnabled = checked;
                            })
                            //.textAlignment(TextFormatAlignment::CENTER)
                        )                           
                    )
                    .child(
                        LayoutBuilder::hBox()
                        .relativeWidth(1.0f)
                        .absoluteHeight(100.0f)
                        //.padding({10, 10, 10, 10}) 

                        .child(
                                    LayoutBuilder::label(
                                        Localization::Translation::fromKeyToWstring(
                                            "Ui.Editor.ColorGrading.LUTexture"
                                        )
                                    )
                            .relativeHeight(1.0f)
                            .relativeWidth(0.4f)
                        )
                        .child(
                            LayoutBuilder::widget(new Widgets::MaterialWidget())
                            .relativeHeight(1.0f)
                            .relativeWidth(0.6f)
                        )
                    )
            )
            .child(
                LayoutBuilder::section(
                    Localization::Translation::fromKeyToWstring("Ui.Editor.SSR")
                )
                        .relativeWidth(1.0f)
                        .autoHeight()
                        .style(
                            [this](auto& s)
                            {
                                s.color = {52, 52, 52};
                            }
                        )
                        .margin({0, 10, 10, 10})
                        .child(
                            LayoutBuilder::vBox()
                                .relativeWidth(1.0f)
                                .absoluteHeight(30.0f)
                                .child(
                                    LayoutBuilder::widget(new Widgets::CheckBox())
                                        .relativeHeight(1.0f)
                                        .relativeWidth(1.0f)
                                        .text(
                                            Localization::Translation::fromKeyToWstring(
                                                "Ui.Editor.SSR.Enabled"
                                            )
                                        )
                                        .apply<Widgets::CheckBox>(
                                            [](Widgets::CheckBox* c)
                                            {
                                                c->setChecked(false);

                                            }
                                        )
                                        .onEvent(
                                            &Widgets::CheckBox::onCheckStateChanged,
                                            [&](bool checked)
                                            {
                                                engine->getRenderer()
                                                    ->getPostProcessConfig()
                                                    .isSSREnabled = checked;
                                            }
                                        )
                                    //.textAlignment(TextFormatAlignment::CENTER)
                                )
                        )
                        
            )

            .child(LayoutBuilder::spacer())
            .build();

    debugWindow->getLayoutRoot()->addChild(std::move(ui));
}

void EditorApp::openFilePicker(
    const std::wstring&                     title,
    std::function<void(const std::string&)> onSelected,
    Win32Window::IWindow*                   parent,
    const std::vector<std::string>&         extentions
)
{
    if (filePickerWindow)
    {
        filePickerWindow = nullptr;
    }

    NbSize<int> winSize = {500, 600};
    auto newWin = std::make_shared<Win32Window::ModalWindow>(
        winSize, parent ? parent : inspectorWindow.get()
    );

    filePickerWindow = newWin;
    newWin->setTitle(title);

    std::weak_ptr<Win32Window::ModalWindow> weakWin = newWin;

    auto ui = LayoutBuilder::vBox()
                  .style(
                      [](auto& s)
                      {
                          s.color = {40, 40, 40};
                      }
                  )
                  .child(
                      LayoutBuilder::widget(new Widgets::FilePicker({0, 0, 500, 600}, extentions))
                          .relativeWidth(1.0f)
                          .relativeHeight(1.0f)
                          .onEvent(
                              &Widgets::FilePicker::onFileSelected,
                              [this, onSelected, weakWin](const std::string& newPath)
                              {
                                  if (auto pinnedWin = weakWin.lock())
                                  {
                                      if (!newPath.empty() && onSelected)
                                      {
                                          onSelected(newPath);
                                      }
                                      PostMessage(
                                          (HWND)pinnedWin->getHandle().as<HWND>(), WM_CLOSE, 0, 0
                                      );
                                  }
                              }
                          )
                          .onEvent(
                              &Widgets::FilePicker::onCancelButtonPressed,
                              [weakWin]()
                              {
                                  if (auto pinnedWin = weakWin.lock())
                                  {
                                      PostMessage(
                                          (HWND)pinnedWin->getHandle().as<HWND>(), WM_CLOSE, 0, 0
                                      );
                                  }
                              }
                          )
                  )
                  .build();

    newWin->getLayoutRoot()->addChild(std::move(ui));
    newWin->show();
}

nbui::LayoutBuilder EditorApp::createMenuButton(
    const std::string&                    labelKey,
    std::function<void(nbui::PopupMenu*)> populateMenuFunc
) noexcept
{
    using namespace nbui;
    return LayoutBuilder::widget(new Widgets::Button())
        .text(Localization::Translation::fromKeyToWstring(labelKey))
        .absoluteWidth(60.0f)
        .relativeHeight(1.0f)
        .apply<Widgets::Button>(
            [this, populateMenuFunc](Widgets::Button* btn)
            {
                subscribe(
                    btn, &Widgets::Button::onReleasedSignal,
                    [this, btn, populateMenuFunc]()
                    {
                        auto popup = new PopupMenu();
                        populateMenuFunc(popup);

                        const NbRect<int>&                pt    = btn->getRect();
                        const WindowInterface::FrameSize& frame = mainWindow->getFrameSize();

                        toolbarWindow->getPopupManager().show(
                            popup, frame.left + pt.x, frame.top + pt.y + pt.height,
                            PopupStyle::MenuBarItem
                        );
                    }
                );
            }
        );
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


    if (sceneController->getActiveNode().getId() == 0)
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
    auto entityId = sceneController->getActiveNode().getId();

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
                                       LayoutBuilder::label(Localization::Translation::fromKeyToWstring(info->name))
                                           .relativeWidth(1.0f)
                                           .absoluteHeight(30)
                                           .background({60, 60, 60})
                                           .color({220, 220, 220})
                                           .textAlignment({.textAlignment = TextAlignment::CENTER})
                                   );

            for (const auto& field : info->fields)
            {
                inspectorBuilder = buildFieldUI(std::move(inspectorBuilder), data, info, field);
            }
            inspectorBuilder =
                std::move(inspectorBuilder).child(LayoutBuilder::spacerAbsolute(1.0f, 5.0f));

        }
    }

    if (sceneController->getActiveNode().isValid())
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
                            auto  entityId = sceneController->getActiveNode().getId();

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
                        .text(Localization::Translation::fromKeyToWstring("Ui.Editor.AddComponent"))
            );
    }



    auto finalUi = std::move(inspectorBuilder).build();

    inspectorWindow->getLayoutRoot()->clearChilds();
    inspectorWindow->getLayoutRoot()->addChild(std::move(finalUi));
    //inspectorWindow->show();
}

void EditorApp::subscribeAll() noexcept
{
    subscribe(this, &EditorApp::onActiveNodeChanged, [&]() {
            shouldRebuildInspector = true;
            engine->setEditorSelectedNode(sceneController->getActiveNode());
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
            LayoutBuilder::label(
                Localization::Translation::fromKeyToWstring(field.name)
            )
                .relativeWidth(0.35f)
                .color({180, 180, 180})
                .padding({0,0,0,5})
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
            LayoutBuilder::label(Localization::Translation::fromKeyToWstring(field.name))
                .relativeWidth(0.35f)
                .color({180, 180, 180})
                .padding({0,0,0,5})
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
                    LayoutBuilder::label(Localization::Translation::fromKeyToWstring(field.name))
                        .relativeWidth(0.35f)
                        .color({180, 180, 180})
                        .padding({0, 0, 0, 5})
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

                                        if (colorPickerWindow)
                                        {
                                            colorPickerWindow = nullptr;
                                        }

                                        NbSize<int> size = {300, 600};

                                        auto newWin = std::make_shared<Win32Window::ModalWindow>(
                                            size,
                                            inspectorWindow.get()
                                        );
                                        colorPickerWindow = newWin; 

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
                                LayoutBuilder::label(
                                    Localization::Translation::fromKeyToWstring(field.name)
                                )
                                    .relativeWidth(0.35f)
                                    .padding({0, 0, 0, 5})
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
                               LayoutBuilder::label(
                                   Localization::Translation::fromKeyToWstring(field.name)
                               )
                                   .relativeWidth(0.35f)
                                   .color({180, 180, 180})
                                   .padding({0, 0, 0, 5})
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

        const int slotHeight   = 100; 
        const int headerHeight = 30; 

        auto vectorColumn = LayoutBuilder::vBox().relativeWidth(1.0f).absoluteHeight(
            (int)vecPtr->size() * slotHeight + headerHeight
        );

        vectorColumn = std::move(vectorColumn).child(
            LayoutBuilder::label(Localization::Translation::fromKeyToWstring(field.name))
                .relativeWidth(1.0f)
                .absoluteHeight(headerHeight)
                .color({150, 150, 150})
                .padding({0, 0, 0, 5})
                .textAlignment({.textAlignment = TextAlignment::LEFT})
        );

        for (size_t i = 0; i < vecPtr->size(); ++i)
        {
            auto& materialRef = (*vecPtr)[i];
            if (!materialRef) 
            {
                // Пропускаем или рисуем пустой слот, если указатель пуст
                continue; 
            }

            std::string  materialPath = materialRef->getPath();
            std::wstring fullPath     = nb::Utils::toWString(materialPath);
            std::wstring fileName     = fullPath;
            size_t       lastSlash    = fileName.find_last_of(L"/\\");
            if (lastSlash != std::wstring::npos)
            {
                fileName = fileName.substr(lastSlash + 1);
            }

            // Формируем имя файла кэша
            std::string replacedPath = materialPath;
            std::replace(replacedPath.begin(), replacedPath.end(), '/', '_');
            std::string cachePngPath = "Assets/cache/" + replacedPath + ".png";

            // Проверяем наличие превью без постоянного обращения к диску
            bool hasPreview = false;
            if (m_existingPreviews.contains(cachePngPath))
            {
                hasPreview = true;
            }
            else if (std::filesystem::exists(cachePngPath))
            {
                m_existingPreviews.insert(cachePngPath);
                hasPreview = true;
            }

            // Если превью нет на диске и оно еще не генерируется
            if (!hasPreview && !m_pendingPreviews.contains(materialPath))
            {
                m_pendingPreviews.insert(materialPath);

                // Запускаем генерацию в фоновом потоке
                std::thread([this, materialPath, cachePngPath]() {
                try 
                {
                    // Выполняем тяжелую генерацию в фоне
                    nb::Renderer::Renderer::generatePreviewForMaterial(materialPath);

                    // После завершения возвращаем задачу в UI-поток через Engine
                    this->engine->invokeAsync([this, materialPath, cachePngPath](auto&) {
                        m_pendingPreviews.erase(materialPath);
                        m_existingPreviews.insert(cachePngPath);
                        
                        // Сигнализируем о необходимости мягко перерисовать инспектор
                        this->shouldRebuildInspector = true;
                    });
                }
                catch (...)
                {
                    // В случае ошибки обязательно очищаем состояние в UI-потоке
                    this->engine->invokeAsync([this, materialPath](auto&) {
                        m_pendingPreviews.erase(materialPath);
                    });
                }
            }).detach(); // .detach() освобождает поток, и он больше не блокирует UI
            }

            // Отрисовка виджета
            vectorColumn = std::move(vectorColumn).child(
                LayoutBuilder::hBox()
                    .relativeWidth(1.0f)
                    .absoluteHeight(slotHeight)
                    .margin({0, 2, 0, 2}) 
                    .child(
                        LayoutBuilder::label(
                            Localization::Translation::fromKeyToWstring("Slot") + std::to_wstring(i)
                        )
                            .relativeWidth(0.35f)
                            .color({100, 100, 100})
                            .padding({0, 0, 0, 5})
                    )
                    .child(
                        LayoutBuilder::widget(new Widgets::MaterialWidget())
                            .relativeWidth(0.65f)
                            .absoluteHeight(slotHeight)
                            .background({45, 45, 45})
                            .apply<Widgets::MaterialWidget>(
                                [fileName, materialPath](Widgets::MaterialWidget* w)
                                {
                                    w->setMaterial(
                                        fileName,
                                        materialPath,
                                        true
                                    );
                                }
                            )
                            .onEvent(
                                &Widgets::MaterialWidget::onClickSignal,
                                [this, materialRef]()
                                {
                                    if (materialEditor)
                                    {
                                        materialEditor = nullptr;
                                    }

                                    materialEditor = std::make_shared<MaterialEditor>(
                                        debugWindow.get(), engine.get(),
                                        nbstl::NonOwningPtr(materialRef.get())
                                    );

                                    materialEditor->show();
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
                    LayoutBuilder::label(Localization::Translation::fromKeyToWstring(field.name))
                        .relativeWidth(0.35f)
                        .color({180, 180, 180})
                        .padding({0,0,0,5})
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
                                        openFilePicker(
                                            L"Select Resource: " + nb::Utils::toWString(field.name),
                                            [this, field, fieldData, componentPtr,
                                             info](const std::string& path)
                                            {
                                                if (field.loadResource)
                                                {
                                                    field.loadResource(fieldData, path);
                                                    markComponentDirty(componentPtr, info);
                                                    shouldRebuildInspector = true;
                                                }
                                            },
                                            inspectorWindow.get()
                                        );
                                    }
                                )
                        )
                );

        return std::move(parentBuilder).child(std::move(resourceRow));
    }
    else if (typeName.find("std::filesystem::path") != std::string::npos)
    {
        std::string  path     = (*static_cast<std::filesystem::path*>(fieldData)).string();
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
                    LayoutBuilder::label(Localization::Translation::fromKeyToWstring(field.name))
                        .relativeWidth(0.35f)
                        .color({180, 180, 180})
                        .padding({0, 0, 0, 5})
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
                                .margin({2, 2, 2, 2})
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
                                        openFilePicker(
                                            L"Select Resource: " + nb::Utils::toWString(field.name),
                                            [this, field, fieldData, componentPtr,
                                             info](const std::string& path)
                                            {
                                               
                                                *static_cast<std::filesystem::path*>(fieldData) = path;
                                                markComponentDirty(componentPtr, info);
                                                shouldRebuildInspector = true;
                                                
                                            },
                                            inspectorWindow.get()
                                        );
                                    }
                                )
                        )
                );

        return std::move(parentBuilder).child(std::move(resourceRow));
    }
    else if (field.type->isVector)
    {
        size_t size = field.type->vectorSize(fieldData);
        nb::Reflect::TypeInfo* elemType = field.type->elementType;

        const int itemHeight = 32;
        const int headerHeight = 35;

        auto vectorColumn = LayoutBuilder::vBox()
            .relativeWidth(1.0f)
            .absoluteHeight((int)size * itemHeight + headerHeight);

        auto headerRow = LayoutBuilder::hBox()
            .relativeWidth(1.0f)
            .absoluteHeight(headerHeight);

        headerRow = std::move(headerRow).child(
            LayoutBuilder::label(Localization::Translation::fromKeyToWstring(field.name) + L" [" + std::to_wstring(size) + L"]")
                .relativeWidth(0.75f)
                .color({150, 150, 150})
                .padding({0, 0, 0, 5})
                .textAlignment({.textAlignment = TextAlignment::LEFT})
        );

        if (field.type->vectorPushBackDefault)
        {
            headerRow = std::move(headerRow).child(
                LayoutBuilder::widget(new Widgets::Button())
                    .text(L"➕")
                    .absoluteWidth(30)
                    .absoluteHeight(25)
                    .background({50, 50, 50})
                    .onEvent(&Widgets::Button::onReleasedSignal, [this, fieldData, componentPtr, info, field]() {
                        field.type->vectorPushBackDefault(fieldData);
                        markComponentDirty(componentPtr, info);
                        this->shouldRebuildInspector = true;
                    })
            );
        }

        vectorColumn = std::move(vectorColumn).child(std::move(headerRow));

        for (size_t i = 0; i < size; ++i)
        {
            void* elemPtr = field.type->vectorAt(fieldData, i);

            auto row = LayoutBuilder::hBox()
                .relativeWidth(1.0f)
                .absoluteHeight(itemHeight)
                .margin({0, 1, 0, 1});

            // --- Динамическое определение заголовка строки ---
            std::wstring itemLabel = L"  [" + std::to_wstring(i) + L"]";
            bool isNamedPair = false;

            if (elemType->isPair && elemType->fields.size() >= 2)
            {
                auto& firstField = elemType->fields[0];
                // Если первый элемент пары — строка, используем её значение как имя
                if (std::strcmp(firstField.type->name, "std::string") == 0)
                {
                    void* firstFieldPtr = (char*)elemPtr + firstField.offset;
                    std::string strKey = *static_cast<std::string*>(firstFieldPtr);
                    if (!strKey.empty())
                    {
                        itemLabel = L"  " + nb::Utils::toWString(strKey);
                        isNamedPair = true; // Помечаем, что это пара с текстовым ключом
                    }
                }
            }

            row = std::move(row).child(
                LayoutBuilder::label(itemLabel)
                    .absoluteWidth(100) // Увеличенная ширина под имя переменной
                    .color({160, 160, 160})
                    .textAlignment({.textAlignment = TextAlignment::LEFT})
            );

            std::string elemTypeName = elemType->name;

            if (elemTypeName == "float")
            {
                row = std::move(row).child(
                    LayoutBuilder::widget(new Widgets::FloatSpinBox())
                        .apply<Widgets::FloatSpinBox>([this, elemPtr, componentPtr, info, field](Widgets::FloatSpinBox* c) {
                            c->setStep(field.step);
                            c->bind(
                                [elemPtr]() { return *static_cast<float*>(elemPtr); },
                                [this, elemPtr, componentPtr, info](float v) {
                                    *static_cast<float*>(elemPtr) = v;
                                    markComponentDirty(componentPtr, info);
                                }
                            );
                        })
                        .relativeWidth(0.7f)
                        .absoluteHeight(28)
                        .background({25, 25, 25})
                );
            }
            else if (elemTypeName == "int" || elemTypeName == "int32_t")
            {
                row = std::move(row).child(
                    LayoutBuilder::widget(new Widgets::FloatSpinBox())
                        .apply<Widgets::FloatSpinBox>([this, elemPtr, componentPtr, info](Widgets::FloatSpinBox* c) {
                            c->setStep(1.0f);
                            c->bind(
                                [elemPtr]() { return static_cast<float>(*static_cast<int32_t*>(elemPtr)); },
                                [this, elemPtr, componentPtr, info](float v) {
                                    *static_cast<int32_t*>(elemPtr) = static_cast<int32_t>(v);
                                    markComponentDirty(componentPtr, info);
                                }
                            );
                        })
                        .relativeWidth(0.7f)
                        .absoluteHeight(28)
                        .background({25, 25, 25})
                );
            }
            else if (elemTypeName == "bool")
            {
                row = std::move(row).child(
                    LayoutBuilder::widget(new Widgets::CheckBox())
                        .apply<Widgets::CheckBox>([this, elemPtr, componentPtr, info](Widgets::CheckBox* c) {
                            c->setChecked(*static_cast<bool*>(elemPtr));
                            c->onToggled([this, elemPtr, componentPtr, info](bool v) {
                                *static_cast<bool*>(elemPtr) = v;
                                markComponentDirty(componentPtr, info);
                            });
                        })
                        .relativeWidth(0.7f)
                        .absoluteHeight(28)
                );
            }
            else if (!elemType->fields.empty() || elemType->isPair)
            {
                auto fieldsBox = LayoutBuilder::hBox().relativeWidth(0.72f);
                
                // Если это именованная пара, первый элемент (имя) пропускаем при отрисовке полей ввода,
                // начиная сразу со второго элемента (значения)
                size_t startIndex = isNamedPair ? 1 : 0;
                size_t fieldsCount = elemType->fields.size();

                for (size_t fIdx = startIndex; fIdx < fieldsCount; ++fIdx)
                {
                    const auto& subField = elemType->fields[fIdx];
                    void* subFieldData = (char*)elemPtr + subField.offset;
                    std::string subTypeName = subField.type->name;

                    auto subColumn = LayoutBuilder::vBox()
                        .relativeWidth(1.0f / (fieldsCount - startIndex))
                        .autoHeight();

                    if (subTypeName == "float")
                    {
                        subColumn = std::move(subColumn).child(
                            LayoutBuilder::widget(new Widgets::FloatSpinBox())
                                .apply<Widgets::FloatSpinBox>([this, subFieldData, componentPtr, info, subField](Widgets::FloatSpinBox* c) {
                                    c->setStep(subField.step);
                                    c->bind(
                                        [subFieldData]() { return *static_cast<float*>(subFieldData); },
                                        [this, subFieldData, componentPtr, info](float v) {
                                            *static_cast<float*>(subFieldData) = v;
                                            markComponentDirty(componentPtr, info);
                                        }
                                    );
                                })
                                .relativeWidth(0.95f)
                                .absoluteHeight(28)
                                .background({25, 25, 25})
                        );
                    }
                    else if (subTypeName == "int" || subTypeName == "int32_t")
                    {
                        subColumn = std::move(subColumn).child(
                            LayoutBuilder::widget(new Widgets::FloatSpinBox())
                                .apply<Widgets::FloatSpinBox>([this, subFieldData, componentPtr, info](Widgets::FloatSpinBox* c) {
                                    c->setStep(1.0f);
                                    c->bind(
                                        [subFieldData]() { return static_cast<float>(*static_cast<int32_t*>(subFieldData)); },
                                        [this, subFieldData, componentPtr, info](float v) {
                                            *static_cast<int32_t*>(subFieldData) = static_cast<int32_t>(v);
                                            markComponentDirty(componentPtr, info);
                                        }
                                    );
                                })
                                .relativeWidth(0.95f)
                                .absoluteHeight(28)
                                .background({25, 25, 25})
                        );
                    }

                    fieldsBox = std::move(fieldsBox).child(std::move(subColumn));
                }
                row = std::move(row).child(std::move(fieldsBox));
            }

            // Кнопка удаления элемента
            row = std::move(row).child(
                LayoutBuilder::widget(new Widgets::Button())
                    .text(L"❌")
                    .absoluteWidth(25)
                    .absoluteHeight(25)
                    .background({40, 40, 40})
                    .onEvent(&Widgets::Button::onReleasedSignal, [this, fieldData, componentPtr, info, field]() {
                        size_t currentSize = field.type->vectorSize(fieldData);
                        if (currentSize > 0)
                        {
                            field.type->vectorResize(fieldData, currentSize - 1);
                            markComponentDirty(componentPtr, info);
                            this->shouldRebuildInspector = true;
                        }
                    })
            );

            vectorColumn = std::move(vectorColumn).child(std::move(row));
        }

        return std::move(parentBuilder).child(std::move(vectorColumn));
    }

    return parentBuilder;
}

void EditorApp::refreshInterfaceText() noexcept
{
    using namespace Localization;

    if (mainWindow)
    {
        mainWindow->setTitle(Translation::fromKeyToWstring("Ui.Editor.Title"));
    }

    if (sceneWindow)
    {
        sceneWindow->getTabWindow()->setTitle(Translation::fromKeyToWstring("Ui.Editor.Scene.Title"));
    }

    if (hierarchyWindow)
    {
        hierarchyWindow->setTitle(Translation::fromKeyToWstring("Ui.Editor.Hierarchy.Title"));
    }

    if (inspectorWindow)
    {
        inspectorWindow->setTitle(Translation::fromKeyToWstring("Ui.Editor.Inspector.Title"));
    }

    if (debugWindow)
    {
        debugWindow->setTitle(Translation::fromKeyToWstring("Ui.Editor.DebugWindow.Title"));
    }

    if (toolbarWindow)
    {
        toolbarWindow->getLayoutRoot()->clearChilds();
        setupMainWindow(); 
    }

    if (debugWindow)
    {
        debugWindow->getLayoutRoot()->clearChilds();
        setupDebugUI();
    }

    if (sceneController->getActiveNode().isValid())
    {
        rebuildInspector();
    }

    refreshHierarchyTreeViewSignal.emit();

    if (assetManager)
    {
        assetManager->setTitle(Translation::fromKeyToWstring("Ui.AssetBrowser.Title"));
    }
}

void EditorApp::setupHierarchyEvents(Widgets::TreeView* tv) noexcept
{
    using namespace nbui;
    subscribe(
        this, &EditorApp::refreshHierarchyTreeViewSignal,
        [this, tv]()
        {
            tv->refresh();
        }
    );


    subscribe(
        tv, &Widgets::TreeView::onItemClickSignal,
        [this](const auto& index)
        {
            if (!index.isValid())
            {
                return;
            }

            if (auto* item = sceneModel->findById(index.getUuid()))
            {
                const auto id = reinterpret_cast<nb::Ecs::EntityID>(item->getData());
                sceneController->setActiveNode(nb::Scene::getInstance().getNode(id));
                onActiveNodeChanged.emit();
            }
        }
    );

    subscribe(
        this, &EditorApp::onActiveNodeChanged,
        [this, tv]()
        {
            if (!sceneController->getActiveNode().isValid() || !sceneModel)
            {
                return;
            }

            const auto targetEntityId = sceneController->getActiveNode().getId();

            Widgets::ModelIndex foundIndex;

            sceneModel->forEach(
                [&](const Widgets::ModelItem& item)
                {
                    if (reinterpret_cast<nb::Ecs::EntityID>(item.getData()) == targetEntityId)
                    {
                        foundIndex = Widgets::ModelIndex(item.getUuid());
                    }
                }
            );

            if (foundIndex.isValid())
            {
                tv->setSelectedItem(foundIndex);
            }
        }
    );

    subscribe(
        tv, &Widgets::TreeView::onItemRightClickSignal,
        [this, tv](const auto& index)
        {
            auto* popup = new PopupMenu();

            auto addPrimitiveAction = [&](const wchar_t* label, const char* paramsType)
            {
                popup->addItem(
                    label, nbui::IconType::Plus,
                    [this, index, paramsType]()
                    {
                        auto* dialog = new PrimitiveCreationDialog(
                            inspectorWindow.get(), paramsType,
                            [this, index](void* data, nb::Reflect::TypeInfo* typeInfo)
                            {
                                sceneController->spawnPrimitive(index, data, typeInfo);
                            }
                        );
                        dialog->show();
                    }
                );
            };

            popup->addItem(
                L"➕ Добавить пустышку", nbui::IconType::Plus,
                [this, index]()
                {
                    sceneController->spawnEmpty(index);
                }
            );

            addPrimitiveAction(L"➕ Добавить куб", "CubeParams");
            addPrimitiveAction(L"➕ Добавить сферу", "SphereParams");
            addPrimitiveAction(L"➕ Добавить тор", "TorusParams");
            addPrimitiveAction(L"➕ Добавить цилиндр", "CylinderParams");
            addPrimitiveAction(L"➕ Добавить плоскость", "PlaneParams");
            addPrimitiveAction(L"➕ Добавить конус", "ConeParams");
            addPrimitiveAction(L"➕ Добавить пирамиду", "PyramidParams");

            popup->addSeparator();

            popup->addItem(
                L"✏️ Переименовать", nbui::IconType::Edit,
                [tv, index]()
                {
                    tv->startEditing(index);
                }
            );

            popup->addItem(
                L"🗑️ Удалить", nbui::IconType::Delete,
                [this, index]()
                {
                    sceneController->deleteEntity(index);

                    nb::Error::ErrorManager::instance().report(
                        nb::Error::Type::INFO, "Delete requested"
                    );
                }
            );
            popup->addSeparator();


            popup->addItem(
                L"Копировать", nbui::IconType::None,
                [this, index]()
                {
                    sceneController->copyEntity(index);
                }
            );

            popup->addItem(
                L"Вставить", nbui::IconType::None,
                [this, index]()
                {
                    sceneController->pasteEntity(index);
                }
            );

            const auto mousePos = this->mainWindow->getMousePosition();
            this->hierarchyWindow->getPopupManager().show(popup, mousePos.x, mousePos.y);
        }
    );
}

void EditorApp::releaseNamesRecursive(nb::Ecs::EntityID id) noexcept
{
    auto& scene = nb::Scene::getInstance();

    // Если у сущности есть имя, разбираем его и возвращаем индекс в пул
    if (scene.hasComponent<NameComponent>(id))
    {
        const std::string& name = scene.getComponent<NameComponent>(id).name;
        primitiveNameManager.releaseName(name);
    }

    if (scene.hasComponent<HierarchyComponent>(id))
    {
        const auto& hierarchy = scene.getComponent<HierarchyComponent>(id);
        for (auto childId : hierarchy.children)
        {
            releaseNamesRecursive(childId);
        }
    }
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
