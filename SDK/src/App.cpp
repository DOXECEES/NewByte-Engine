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
    assetManager->setOnFileDropCallback([this](const std::filesystem::path& dropFilePath) {

        //if (importWindow)
        //{
        //    importWindow = nullptr;
        //}

        importWindow = std::make_shared<ImportWindow>(nullptr, engine.get(), dropFilePath, [this]() {
            assetManagerWindow->refreshModel();
            importWindow = nullptr;
        });

    });

    
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
    previewWindow = std::make_shared<Win32Window::ChildWindow>(nullptr);
    previewWindow->setTitle(L"prev");
    previewWindow->addCaption();
    previewWindow->setRenderable(false);
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
                LayoutBuilder::hBox()
                    .absoluteHeight(30.0f) 
                    .relativeWidth(1.0f)
                    .style(
                        [](NNsLayout::LayoutStyle& s)
                        {
                            s.color = NbColor{45, 45, 45}; 
                        }
                    )
                    .child(
                        LayoutBuilder::widget(new Widgets::Button())
                            .text(Localization::Translation::fromKeyToWstring("Ui.Editor.File"))
                            .absoluteWidth(60.0f)
                            .relativeHeight(1.0f)
                            .apply<Widgets::Button>(
                                [this](Widgets::Button* btn)
                                {
                                    subscribe(
                                        btn, &Widgets::Button::onReleasedSignal,
                                        [this, btn]()
                                        {
                                            auto popup = new PopupMenu();
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
                                                                    fs::create_directories(
                                                                        p.parent_path()
                                                                    );
                                                                }

                                                                engine->saveSnapshot();
                                                                engine->clearScene();
                                                                sceneModel->rebuildFromScene();
                                                                activeNode = nb::Node::createInvalid();

                                                                refreshHierarchyTreeViewSignal
                                                                    .emit();
                                                                onActiveNodeChanged.emit();
                                                                nb::Scene::getInstance()
                                                                    .invalidateBvh();

                                                                engine->setProjectPath(p.string());
                                                                engine->saveSnapshot(); 

                                                                nb::Error::ErrorManager::instance()
                                                                    .report(
                                                                        nb::Error::Type::INFO,
                                                                        "New project created at: " +
                                                                            p.string()
                                                                    );

                                                                shouldRebuildInspector = true;

                                                                engine->loadSnapshot();
                                                            }
                                                            catch (const std::exception& e)
                                                            {
                                                                nb::Error::ErrorManager::instance()
                                                                    .report(
                                                                        nb::Error::Type::FATAL,
                                                                        std::string(
                                                                            "Failed to create "
                                                                            "project: "
                                                                        ) + e.what()
                                                                    );
                                                            }
                                                        },
                                                        toolbarWindow.get(),
                                                        {".json"}
                                                    );
                                                }
                                            );
                                            popup->addItem(
                                                Localization::Translation::fromKeyToWstring(
                                                    "Ui.Editor.Open"
                                                ),
                                                IconType::None,
                                                [this]()
                                                {
                                                    openFilePicker(
                                                        Localization::Translation::fromKeyToWstring(
                                                            "Ui.Editor.SelectResource"
                                                        ),
                                                        [this](const std::string& path)
                                                        {
                                                            nb::Error::ErrorManager::instance()
                                                                .report(
                                                                    nb::Error::Type::INFO, path
                                                                );

                                                            engine->saveSnapshot();
                                                            engine->clearScene();
                                                            
                                                            engine->setProjectPath(path);
                                                            engine->loadSnapshot();
                                                            
                                                            sceneModel->rebuildFromScene();
                                                            activeNode = nb::Node::createInvalid();

                                                            refreshHierarchyTreeViewSignal.emit();
                                                            onActiveNodeChanged.emit();
                                                            nb::Scene::getInstance()
                                                                .invalidateBvh();
                                                        },
                                                        toolbarWindow.get(), {".json"}
                                                    );

                                                }
                                            );
                                            popup->addSeparator();
                                            popup->addItem(
                                                Localization::Translation::fromKeyToWstring(
                                                    "Ui.Editor.Save"
                                                ),
                                                IconType::None,
                                                [this]()
                                                {
                                                    engine->saveSnapshot();
                                                }
                                            );
                                            popup->addSeparator();
                                            popup->addItem(
                                                Localization::Translation::fromKeyToWstring(
                                                    "Ui.Editor.Exit"
                                                ),
                                                IconType::Delete,
                                                [this]()
                                                {
                                                    PostQuitMessage(0);
                                                }
                                            );

                                            const NbRect<int>& pt = btn->getRect();
                                            const WindowInterface::FrameSize& frame =
                                                mainWindow->getFrameSize();

                                            toolbarWindow->getPopupManager().show(
                                                popup, frame.left + pt.x, frame.top + pt.y + pt.height, PopupStyle::MenuBarItem
                                            );
                                        }
                                    );
                                }
                            )
                    )
                    .child(
                        LayoutBuilder::widget(new Widgets::Button())
                            .text(Localization::Translation::fromKeyToWstring("Ui.Editor.Edit"))
                            .absoluteWidth(60.0f)
                            .relativeHeight(1.0f)
                            .apply<Widgets::Button>(
                                [this](Widgets::Button* btn)
                                {
                                    subscribe(
                                        btn, &Widgets::Button::onReleasedSignal,
                                        [this, btn]()
                                        {
                                            auto popup = new PopupMenu();
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
                                                            nb::Error::ErrorManager::instance()
                                                                .report(
                                                                    nb::Error::Type::INFO, path
                                                                );
                                                        },
                                                        toolbarWindow.get()
                                                    );

                                                    
                                                    //engine->loadSnapshot()
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

                                            const NbRect<int>&                pt = btn->getRect();
                                            const WindowInterface::FrameSize& frame = mainWindow->getFrameSize();

                                            toolbarWindow->getPopupManager().show(
                                                popup, frame.left + pt.x,
                                                frame.top + pt.y + pt.height,
                                                PopupStyle::MenuBarItem
                                            );
                                        }
                                    );
                                }
                            )
                    )
                    .child(
                        LayoutBuilder::widget(new Widgets::Button())
                            .text(Localization::Translation::fromKeyToWstring("Ui.Editor.View"))
                            .absoluteWidth(60.0f)
                            .relativeHeight(1.0f)
                            .apply<Widgets::Button>(
                                [this](Widgets::Button* btn)
                                {
                                    subscribe(
                                        btn, &Widgets::Button::onReleasedSignal,
                                        [this, btn]()
                                        {
                                            auto popup = new PopupMenu();
                                            popup->addItem(
                                                Localization::Translation::fromKeyToWstring(
                                                    "Ui.Editor.View.Lang"
                                                ),
                                                IconType::Plus,
                                                [this]()
                                                {
                                                    if (languagePicker)
                                                    {
                                                        languagePicker = nullptr;
                                                    }

                                                    languagePicker =
                                                        std::make_shared<Win32Window::ModalWindow>(
                                                            NbSize<int>{400,120}, toolbarWindow.get()
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
                                                            .spacing(
                                                                0
                                                            ) 
                                                            .relativeWidth(1.0f)
                                                            .relativeHeight(1.0f)
                                                            .background({35, 35, 35})
                                                            .child(
                                                                LayoutBuilder::hBox()
                                                                    .relativeWidth(1.0f)
                                                                    .absoluteHeight(
                                                                        35
                                                                    ) 
                                                                    .child(
                                                                        LayoutBuilder::label(
                                                                            Localization::
                                                                                Translation::
                                                                                    fromKeyToWstring(
                                                                                        "Ui.Editor."
                                                                                        "Language"
                                                                                    )
                                                                        )
                                                                            .relativeWidth(0.4f)
                                                                            .color({200, 200, 200})
                                                                            .textAlignment(
                                                                                {.textAlignment =
                                                                                     TextAlignment::
                                                                                         LEFT}
                                                                            )
                                                                    )
                                                                    .child(
                                                                        LayoutBuilder::widget(
                                                                            new Widgets::ComboBox()
                                                                        )
                                                                            .relativeWidth(0.6f)
                                                                            .absoluteHeight(30)
                                                                            .apply<
                                                                                Widgets::ComboBox>(
                                                                                [state](
                                                                                    Widgets::
                                                                                        ComboBox* cb
                                                                                )
                                                                                {
                                                                                    cb->addItem(
                                                                                        {L"Русский",
                                                                                         0}
                                                                                    );
                                                                                    cb->addItem(
                                                                                        {L"English",
                                                                                         1}
                                                                                    );

                                                                                    
                                                                                    cb->setSelectedItem(
                                                                                        0
                                                                                    );
                                                                                }
                                                                            )
                                                                            .onEvent(
                                                                                &Widgets::ComboBox::
                                                                                    onSelectionChanged,
                                                                                [state](
                                                                                    const Widgets::
                                                                                        ListItem&
                                                                                            item
                                                                                )
                                                                                {
                                                                                    state
                                                                                        ->selectedLang =
                                                                                        item.getValue<
                                                                                            int>();
                                                                                }
                                                                            )
                                                                    )
                                                            )
                                                            .child(
                                                                LayoutBuilder::spacer()
                                                            ) 
                                                            .child(
                                                                LayoutBuilder::hBox()
                                                                    .relativeWidth(1.0f)
                                                                    .absoluteHeight(35)
                                                                    .spacing(10)
                                                                    .child(
                                                                        LayoutBuilder::spacer()
                                                                            .relativeWidth(0.2f)
                                                                    ) 
                                                                    .child(
                                                                        LayoutBuilder::widget(
                                                                            new Widgets::Button()
                                                                        )
                                                                            .text(
                                                                                Localization::
                                                                                    Translation::
                                                                                        fromKeyToWstring(
                                                                                            "Ui."
                                                                                            "Editor"
                                                                                            ".Save"
                                                                                        )
                                                                            )
                                                                            .relativeWidth(0.4f)
                                                                            .background(
                                                                                {60, 60, 60}
                                                                            )
                                                                            .onEvent(
                                                                                &Widgets::Button::
                                                                                    onReleasedSignal,
                                                                                [this, state]()
                                                                                {
                                                                                    if (state
                                                                                            ->selectedLang ==
                                                                                        0)
                                                                                    {
                                                                                        Localization::
                                                                                            Translation::load(
                                                                                                "As"
                                                                                                "se"
                                                                                                "ts"
                                                                                                "/L"
                                                                                                "oc"
                                                                                                "al"
                                                                                                "iz"
                                                                                                "at"
                                                                                                "io"
                                                                                                "n/"
                                                                                                "ru"
                                                                                                ".t"
                                                                                                "ra"
                                                                                                "ns"
                                                                                                "la"
                                                                                                "ti"
                                                                                                "on"
                                                                                            );
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        Localization::
                                                                                            Translation::load(
                                                                                                "As"
                                                                                                "se"
                                                                                                "ts"
                                                                                                "/L"
                                                                                                "oc"
                                                                                                "al"
                                                                                                "iz"
                                                                                                "at"
                                                                                                "io"
                                                                                                "n/"
                                                                                                "en"
                                                                                                ".t"
                                                                                                "ra"
                                                                                                "ns"
                                                                                                "la"
                                                                                                "ti"
                                                                                                "on"
                                                                                            );
                                                                                    }

                                                                                    this->refreshInterfaceText();
                                                                                    languagePicker
                                                                                        ->close();
                                                                                }
                                                                            )
                                                                    )
                                                                    .child(
                                                                        LayoutBuilder::widget(
                                                                            new Widgets::Button()
                                                                        )
                                                                            .text(
                                                                                Localization::
                                                                                    Translation::
                                                                                        fromKeyToWstring(
                                                                                            "Ui."
                                                                                            "Materi"
                                                                                            "alEdit"
                                                                                            "or."
                                                                                            "Exit"
                                                                                        )
                                                                            )
                                                                            .relativeWidth(0.4f)
                                                                            .background(
                                                                                {50, 50, 50}
                                                                            )
                                                                            .onEvent(
                                                                                &Widgets::Button::
                                                                                    onReleasedSignal,
                                                                                [this]()
                                                                                {
                                                                                    languagePicker
                                                                                        ->close();
                                                                                    languagePicker =
                                                                                        nullptr;
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

                                            const NbRect<int>&                pt = btn->getRect();
                                            const WindowInterface::FrameSize& frame =
                                                mainWindow->getFrameSize();

                                            toolbarWindow->getPopupManager().show(
                                                popup, frame.left + pt.x,
                                                frame.top + pt.y + pt.height,
                                                PopupStyle::MenuBarItem
                                            );
                                        }
                                    );
                                }
                            )
                    )
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
                    .child(
                        LayoutBuilder::widget(new Widgets::Label()).text(L"Main Editor Area")
                    )
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

// Строка с инпутом (Label | Input)
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
    //using namespace nbui;
    //auto debugUI = LayoutBuilder::vBox()
    //    .style([](NNsLayout::LayoutStyle& s) {
    //    s.widthSizeType = NNsLayout::SizeType::ABSOLUTE;
    //    s.width = 250;
    //    s.heightSizeType = NNsLayout::SizeType::RELATIVE;
    //    s.height = 1.0f;
    //    s.color = NbColor{ 35, 35, 35 };
    //    s.padding = { 10, 10, 10, 10 };
    //        })

    //    .child(LayoutBuilder::label(L"VISUALIZATION")
    //        .relativeWidth(1.0f).absoluteHeight(25)
    //        .color(NbColor{ 150, 150, 150 }).fontSize(12))

    //    .child(LayoutBuilder::widget(new Widgets::CheckBox())
    //        .text(L"Wireframe Mode")
    //        .relativeWidth(1.0f).absoluteHeight(30)
    //        .onEvent(&Widgets::CheckBox::onCheckStateChanged, [&](bool checked) {
    //            engine->getRenderer()->setWireframeMode(checked);
    //        }))

    //    .child(LayoutBuilder::widget(new Widgets::CheckBox())
    //        .text(L"Show grid")
    //        .relativeWidth(1.0f).absoluteHeight(30)
    //        .apply<Widgets::CheckBox>(
    //            [](Widgets::CheckBox* checkbox)
    //            {
    //                checkbox->setChecked(true);
    //            }
    //        )
    //        .onEvent(&Widgets::CheckBox::onCheckStateChanged, [&](bool checked) {
    //            engine->getRenderer()->toggleGridShow();
    //        }))

    //    .child(LayoutBuilder::widget(new Widgets::CheckBox())
    //        .text(L"Show light sources")
    //        .relativeWidth(1.0f).absoluteHeight(30)
    //        .onEvent(&Widgets::CheckBox::onCheckStateChanged, [&](bool checked) {
    //            engine->getRenderer()->toggleDebugPass();
    //            }))
    //        .child(
    //            LayoutBuilder::widget(new Widgets::CheckBox())
    //                .text(L"Show BVH bounds")
    //                .relativeWidth(1.0f)
    //                .absoluteHeight(30)
    //                .onEvent(
    //                    &Widgets::CheckBox::onCheckStateChanged,
    //                    [&](bool checked)
    //                    {
    //                        engine->getRenderer()->toggleBvhVisualization();
    //                    }
    //                )
    //        )

    //    // Разделитель
    //    .child(LayoutBuilder::spacer().absoluteHeight(10))

    //    // --- СЕКЦИЯ: ИСТОЧНИКИ СВЕТА ---
    //    .child(LayoutBuilder::label(L"LIGHTING & GIZMOS")
    //        .relativeWidth(1.0f).absoluteHeight(25)
    //        .color(NbColor{ 150, 150, 150 }).fontSize(12))

    //    .child(LayoutBuilder::widget(new Widgets::CheckBox())
    //        .text(L"Show Light Icons")
    //        .relativeWidth(1.0f).absoluteHeight(30)
    //        .onEvent(&Widgets::CheckBox::onCheckStateChanged, [&](bool checked) {
    //                        engine->getRenderer()->toggleSsao();
    //        }))

    //    .child(LayoutBuilder::widget(new Widgets::CheckBox())
    //        .text(L"Show Bounding Boxes")
    //        .relativeWidth(1.0f).absoluteHeight(30)
    //        .onEvent(&Widgets::CheckBox::onCheckStateChanged, [&](bool checked) {
    //                        engine->getRenderer()->toggleBoundingBoxVisualization();
    //        }))

    //    .child(LayoutBuilder::widget(new Widgets::CheckBox())
    //        .text(L"Enable Shadows")
    //        .relativeWidth(1.0f).absoluteHeight(30)
    //        .onEvent(&Widgets::CheckBox::onCheckStateChanged, [](bool checked) {
    //            //g_engine->getRenderer()->setShadowsEnabled(checked);
    //            }))

    //    // Разделитель
    //    .child(LayoutBuilder::spacer().absoluteHeight(10))

    //    // --- СЕКЦИЯ: СТАТИСТИКА ---
    //    .child(LayoutBuilder::label(L"STATISTICS")
    //        .relativeWidth(1.0f).absoluteHeight(25)
    //        .color(NbColor{ 150, 150, 150 }).fontSize(12))



    //    .child(LayoutBuilder::widget(new Widgets::ComboBox())
    //        .apply<Widgets::ComboBox>([&](Widgets::ComboBox* c) {

    //            const nb::Renderer::Renderer* renderer = engine->getRenderer().get();

    //            c->addItem({ L"Albedo",     renderer->getAlbedoId()});
    //            c->addItem({ L"Ao",         renderer->getAoId() });
    //            c->addItem({ L"Metal",      renderer->getMetalId() });
    //            c->addItem({ L"Normal",     renderer->getNormalId()});
    //            c->addItem({ L"Roughtness", renderer->getRoughtnessId()});
    //            c->addItem({ L"Shadow",     renderer->getShadowTextureId() });
    //            c->addItem({ L"Gizmo",      renderer->getGizmoTextureId() });

    //         })
    //        .text(L"Show Draw Calls")
    //        .relativeWidth(1.0f).absoluteHeight(30)
    //        .onEvent(&Widgets::ComboBox::onItemChecked, [&](const Widgets::ListItem& item) {
    //            
    //            engine->getRenderer()->setCheckedTextureId(item.getValue<uint32_t>());
    //        }))
    //    .child(LayoutBuilder::widget(new Widgets::CheckBox())
    //        .text(L"Show FPS Counter")
    //        .relativeWidth(1.0f).absoluteHeight(30)
    //        .onEvent(&Widgets::CheckBox::onCheckStateChanged, [](bool checked) {
    //            //g_engine->getUI()->setOverlayVisible(L"FPS", checked);
    //            }))

    //    .child(LayoutBuilder::spacer()) // Пружина, чтобы все прижалось к верху
    //    .build();
    using namespace nbui;

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
                        .text(Localization::Translation::fromKeyToWstring("Ui.Editor.AddComponent"))
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
            engine->setEditorSelectedNode(activeNode);
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

        // Параметры верстки
        const int slotHeight   = 100; 
        const int headerHeight = 30; 

        auto vectorColumn = LayoutBuilder::vBox().relativeWidth(1.0f).absoluteHeight(
            (int)vecPtr->size() * slotHeight + headerHeight
        );

        vectorColumn = std::move(vectorColumn)
                           .child(
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

            std::wstring fullPath =
                materialRef ? nb::Utils::toWString(materialRef->getPath()) : L"None";
            std::wstring fileName  = fullPath;
            size_t       lastSlash = fileName.find_last_of(L"/\\");
            if (lastSlash != std::wstring::npos)
            {
                fileName = fileName.substr(lastSlash + 1);
            }


            std::string replacedPath = materialRef->getPath();
            std::replace(replacedPath.begin(), replacedPath.end(), '/', '_');
            if (!std::filesystem::exists("Assets/cache/" + replacedPath + ".png"))
            {
                nb::Renderer::Renderer::generatePreviewForMaterial(materialRef->getPath());
            }


            vectorColumn =
                std::move(vectorColumn)
                    .child(
                        LayoutBuilder::hBox()
                            .relativeWidth(1.0f)
                            .absoluteHeight(slotHeight)
                            .margin({0, 2, 0, 2}) 
                            .child(
                                LayoutBuilder::label(
                                    Localization::Translation::fromKeyToWstring("Slot") +
                                    std::to_wstring(i)
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
                                        [fileName, materialRef](Widgets::MaterialWidget* w)
                                        {
                                            w->setMaterial(
                                                fileName,
                                                materialRef->getPath(),
                                                materialRef != nullptr
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

                                            //subscribe(
                                            //    materialEditor.get(),
                                            //    &MaterialEditor::onWindowClose,
                                            //    []()
                                            //    {
                                            //    
                                            //    }
                                            //)

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

    return parentBuilder;
}

void EditorApp::spawnPrimitive(
    const Widgets::ModelIndex& index,
    void*                      data,
    nb::Reflect::TypeInfo*     typeInfo
) noexcept
{
    if (!index.isValid() || !data || !typeInfo)
    {
        nb::Error::ErrorManager::instance()
            .report(nb::Error::Type::WARNING, "Invalid spawn parameters")
            .with("hasData", data != nullptr)
            .with("hasType", typeInfo != nullptr);
        return;
    }

    auto* item = sceneModel->findById(index.getUuid());
    if (!item)
    {
        nb::Error::ErrorManager::instance()
            .report(nb::Error::Type::WARNING, "Could not find scene item by UUID")
            .with("uuid", index.getUuid().toString()); 
        return;
    }

    const auto parentId = reinterpret_cast<nb::Ecs::EntityID>(item->getData());
    auto&      scene    = nb::Scene::getInstance();
    auto       node     = scene.createNode(parentId);

    Ref<nb::Renderer::Mesh> mesh     = nullptr;
    const std::string_view  typeName = typeInfo->name;

    nb::Renderer::Mesh::PrimitiveDescriptor desc;
    desc.type = typeInfo->name;

    for (const auto& field : typeInfo->fields)
    {
        const void*            fieldPtr      = static_cast<const uint8_t*>(data) + field.offset;
        const std::string_view fieldTypeName = field.type->name;

        if (fieldTypeName == "float")
        {
            desc.parameters[field.name] = *static_cast<const float*>(fieldPtr);
        }
        else if (fieldTypeName == "int" || fieldTypeName == "uint32_t")
        {
            desc.parameters[field.name] = static_cast<float>(*static_cast<const int*>(fieldPtr));
        }
    }

    if (typeName == "CubeParams")
    {
        const auto* p = static_cast<CubeParams*>(data);
        mesh = nb::Renderer::PrimitiveGenerators::createCube(p->size);
    }
    else if(typeName == "SphereParams")
    {
        const auto* p = static_cast<SphereParams*>(data);
        mesh = nb::Renderer::PrimitiveGenerators::createSphere(p->radius, p->xSegments, p->ySegments);
    }
    else if (typeName == "TorusParams")
    {
        const auto* p = static_cast<TorusParams*>(data);
        mesh          = nb::Renderer::PrimitiveGenerators::createTorus(
            {static_cast<uint32>(p->xSegments), static_cast<uint32>(p->ySegments)}, p->majorRadius,
            p->minorRadius
        );
    }
    else if (typeName == "CylinderParams")
    {
        const auto* p = static_cast<CylinderParams*>(data);
        mesh          = nb::Renderer::PrimitiveGenerators::createCylinder(
            p->radius, p->height, p->xSegments, p->ySegments
        );
    }
    else if (typeName == "PlaneParams")
    {
        const auto* p = static_cast<PlaneParams*>(data);
        mesh          = nb::Renderer::PrimitiveGenerators::createPlane(
            p->width, p->height, p->xSegments, p->ySegments
        );
    }
    else if (typeName == "ConeParams")
    {
        const auto* p = static_cast<ConeParams*>(data);
        mesh          = nb::Renderer::PrimitiveGenerators::createCone(
            p->radius, p->height, p->radialSegments, p->heightSegments
        );
    }
    else if (typeName == "PyramidParams")
    {
        const auto* p = static_cast<PyramidParams*>(data);
        mesh = nb::Renderer::PrimitiveGenerators::createPyramid(p->radius, p->height, p->sides);
    }

    if (mesh)
    {
        node.addComponent<MeshComponent>({.mesh = mesh, .material = {}});
        std::string primitiveName = primitiveNameManager.generateName(typeName);
        node.addComponent<NameComponent>({primitiveName});

        node.addComponent<TransformComponent>({});

        sceneModel->addEntity(parentId, node.getId());
        
        activeNode = scene.getNode(node.getId());
        refreshHierarchyTreeViewSignal.emit();
        onActiveNodeChanged.emit();
        scene.invalidateBvh();

        nb::Error::ErrorManager::instance()
            .report(nb::Error::Type::INFO, "Primitive spawned successfully")
            .with("type", typeInfo->name)
            .with("name", primitiveName)
            .with("entityId", static_cast<uint64_t>(node.getId()));
    }
    else
    {
        nb::Error::ErrorManager::instance()
            .report(nb::Error::Type::FATAL, "Failed to create mesh for primitive")
            .with("type", typeInfo->name);
    }
}

void EditorApp::spawnEmpty(const Widgets::ModelIndex& index) noexcept
{
    if (!index.isValid())
    {
        nb::Error::ErrorManager::instance().report(
            nb::Error::Type::WARNING, "Invalid spawn parameters for Empty node"
        );
        return;
    }

    auto* item = sceneModel->findById(index.getUuid());
    if (!item)
    {
        nb::Error::ErrorManager::instance()
            .report(nb::Error::Type::WARNING, "Could not find scene item by UUID for Empty node")
            .with("uuid", index.getUuid().toString());
        return;
    }

    const auto parentId = reinterpret_cast<nb::Ecs::EntityID>(item->getData());
    auto&      scene    = nb::Scene::getInstance();
    auto       node     = scene.createNode(parentId);

    std::string nodeName = primitiveNameManager.generateName("Empty");
    node.addComponent<NameComponent>({nodeName});

    node.addComponent<TransformComponent>({});

    sceneModel->addEntity(parentId, node.getId());

    activeNode = scene.getNode(node.getId());

    refreshHierarchyTreeViewSignal.emit();
    onActiveNodeChanged.emit();

    scene.invalidateBvh();

    nb::Error::ErrorManager::instance()
        .report(nb::Error::Type::INFO, "Empty node spawned successfully")
        .with("name", nodeName)
        .with("entityId", static_cast<uint64_t>(node.getId()));
}


void EditorApp::spawnModel(
    const Widgets::ModelIndex&   index,
    const std::filesystem::path& pathToModel,
    const nb::Math::Vector3<float>&         position 
) noexcept
{
    nb::Ecs::EntityID parentId = sceneModel->getRoot();
    

    auto& scene = nb::Scene::getInstance();
    auto  node  = scene.createNode(parentId);

    std::vector<Ref<nb::Resource::MaterialAsset>> materials;
    std::string                                   meshResourcePath = pathToModel.string();

    if (pathToModel.extension() == ".model")
    {
        try
        {
            auto modelJson = nb::Loaders::Json(pathToModel);

            if (modelJson.contains("mesh_source"))
            {
                meshResourcePath = modelJson["mesh_source"].get<std::string>();
            }

            if (modelJson.contains("submeshes"))
            {
                for (int i = 0; i < modelJson["submeshes"].size(); i++)
                {
                    if (modelJson["submeshes"][i].contains("material"))
                    {
                        std::string matPath =
                            modelJson["submeshes"][i]["material"].get<std::string>();
                        auto res = nb::ResMan::ResourceManager::getInstance()
                                       ->getResource<nb::Resource::MaterialAsset>(matPath);
                        if (res)
                        {
                            materials.push_back(res);
                        }
                    }
                }
            }
        }
        catch (const std::exception& e)
        {
            nb::Error::ErrorManager::instance().report(
                nb::Error::Type::WARNING,
                "Failed to load materials from .model: " + std::string(e.what())
            );
        }
    }

    std::string nodeName = primitiveNameManager.generateName(pathToModel.filename().string());
    node.addComponent<NameComponent>({nodeName});

    node.addComponent<TransformComponent>(TransformComponent{.position = position});

    node.addComponent<MeshComponent>(
        {.mesh = nb::ResMan::ResourceManager::getInstance()->getResource<nb::Renderer::Mesh>(
             pathToModel.string()
         ),
         .material = materials}
    );

    sceneModel->addEntity(parentId, node.getId());
    activeNode = scene.getNode(node.getId());

    refreshHierarchyTreeViewSignal.emit();
    onActiveNodeChanged.emit();

    scene.invalidateBvh();

    nb::Error::ErrorManager::instance()
        .report(nb::Error::Type::INFO, "Model spawned successfully")
        .with("name", nodeName);
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
        sceneWindow->setTitle(Translation::fromKeyToWstring("Ui.Editor.Scene.Title"));
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

    if (activeNode.isValid())
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
                activeNode    = nb::Scene::getInstance().getNode(id);
                onActiveNodeChanged.emit();
            }
        }
    );

    subscribe(
        this, &EditorApp::onActiveNodeChanged,
        [this, tv]()
        {
            if (!activeNode.isValid() || !sceneModel)
            {
                return;
            }

            const auto targetEntityId = activeNode.getId();

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
                                this->spawnPrimitive(index, data, typeInfo);
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
                    this->spawnEmpty(index);
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
                    this->deleteEntity(index);

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
                    copyEntity(index);
                }
            );

            popup->addItem(
                L"Вставить", nbui::IconType::None,
                [this, index]()
                {
                    pasteEntity(index);
                }
            );

            const auto mousePos = this->mainWindow->getMousePosition();
            this->hierarchyWindow->getPopupManager().show(popup, mousePos.x, mousePos.y);
        }
    );
}

void EditorApp::deleteEntity(const Widgets::ModelIndex& index) noexcept
{
    if (!index.isValid())
    {
        return;
    }

    nb::Ecs::EntityID id    = sceneModel->getEntity(index);
    auto&             scene = nb::Scene::getInstance();

    releaseNamesRecursive(id);

    sceneModel->removeEntity(id);

    scene.deleteEntity(id);

    activeNode = nb::Node::createInvalid();

    refreshHierarchyTreeViewSignal.emit();
    onActiveNodeChanged.emit();
    scene.invalidateBvh();
}

void EditorApp::copyEntity(const Widgets::ModelIndex& index) noexcept
{
    if (!index.isValid())
    {
        return;
    }

    nb::Ecs::EntityID id    = sceneModel->getEntity(index);
    auto&             scene = nb::Scene::getInstance();

    copiedEntityId = id;

}


void EditorApp::pasteEntity(const Widgets::ModelIndex& index) noexcept
{
    if (!index.isValid())
    {
        return;
    }

    nb::Ecs::EntityID id    = sceneModel->getEntity(index);
    auto&             scene = nb::Scene::getInstance();

    
    
    nb::Node copy = scene.clone(id, copiedEntityId);
    NameComponent& name = copy.getComponent<NameComponent>();
    name.name                += " (Сopy)";
    name.name = primitiveNameManager.generateName(name.name);

    sceneModel->addEntity(id, copy.getId());

    activeNode = copy;

    refreshHierarchyTreeViewSignal.emit();
    onActiveNodeChanged.emit();
    scene.invalidateBvh();
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
