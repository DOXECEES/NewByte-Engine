#include <windows.h>

#include <Win32Window/Win32EventLoop.hpp>
#include <Win32Window/Win32Window.hpp>

#include <Win32Window/Win32ChildWindow.hpp>
#include <Layout.hpp>
#include <DockManager.hpp>


#include <Widgets/Button.hpp>
#include <Widgets/TreeView.hpp>
#include <Widgets/CheckBox.hpp>

#include <Core/Engine.hpp>
#include <Math/Quaternion.hpp>

#include <Core/EngineSettings.hpp>

#include "SceneWindow.hpp"
#include "HierarchyWindow.hpp"
#include "PropertiesWindow.hpp"

#include <thread>

#include <Widgets/Label.hpp>
#include <Widgets/ComboBox.hpp>

HWND activeWindow = nullptr;

HWND hChild;
std::shared_ptr<nb::Core::Engine> g_engine = nullptr;
HWND hwndMain;


#include <atomic>

std::atomic<bool> g_running{ true };
std::atomic<bool> g_input{ false };
std::atomic<bool> g_init{ false };

void engineThreadFunc(nb::Core::Engine* engine, HWND han)
{
    if (g_engine == nullptr)
    {
        g_engine = std::make_shared<nb::Core::Engine>(han);
    }
    while (g_running)
    {
        g_init = true;
        g_engine->processInput();
        g_engine->run({}, {});
        //catchError();
    }
}

class SceneModel : public Widgets::ITreeModel
{
public:
    SceneModel(std::shared_ptr<nb::Renderer::SceneGraph> sceneGraph)
    {
        struct StackItem
        {
            nb::Renderer::BaseNode* node;
            Widgets::ModelItem* modelItem;
            size_t                  depth;
        };

        auto rootNode = sceneGraph->getScene().get();
        rootItems.push_back(std::make_unique<Widgets::ModelItem>(rootNode, nullptr, 0)); // depth of root node = 0
        // addNode(node, parent = nulllptr);
        std::stack<StackItem> stk;
        stk.push({ rootNode, rootItems.back().get(), 0 });

        while (!stk.empty())
        {
            auto& [current, item, depth] = stk.top();
            stk.pop();

            for (const auto& c : current->getChildrens())
            {
                auto childItem = std::make_unique<Widgets::ModelItem>(c.get(), item, depth + 1);
                Widgets::ModelItem* rawPtr = childItem.get();

                item->children.push_back(std::move(childItem));
                // addNode(childItem, item)
                stk.push({ c.get(), rawPtr, depth + 1 });
            }
        }
    }


    const std::vector<std::unique_ptr<Widgets::ModelItem>>& getChildren(const Widgets::ModelItem& parent) const noexcept override
    {
        return parent.children;
    }

    bool hasChildren(const Widgets::ModelItem& parent) const noexcept override
    {
        return !parent.children.empty();
    }

    void forEach(std::function<void(const Widgets::ModelItem&)> func) noexcept override
    {
        std::stack<const Widgets::ModelItem*> stk;

        for (const auto& i : rootItems)
        {
            stk.push(i.get());
        }

        while (!stk.empty())
        {
            const Widgets::ModelItem* item = stk.top();
            stk.pop();

            func(*item);

            for (const auto& child : item->children)
            {
                stk.push(child.get());
            }
        }
    }


    std::string data(const Widgets::ModelItem& item) const noexcept override
    {
        nb::Renderer::BaseNode* n = static_cast<nb::Renderer::BaseNode*>(item.data);
        return n->getName();
    }

    std::string data(const Widgets::ModelIndex& index) noexcept override
    {
        std::optional<std::reference_wrapper<const Widgets::ModelItem>> item = findByIndex(index);
        if (!item.has_value())
        {
            return {};
        }
        nb::Renderer::BaseNode* n = static_cast<nb::Renderer::BaseNode*>(item->get().data);
        return n->getName();
    }

private:


};


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

    const wchar_t* mainClassName = L"MDIMainWindow";
    AllocConsole();

    Win32Window::Window window;
    window.setTitle(L"NewByte SDK");
    Win32Window::Win32EventLoop eventLoop;

    Win32Window::ChildWindow sceneWindow(&window);

    Win32Window::ChildWindow childWnd(&window);
    // childWnd.addCaption();
    childWnd.setTitle(L"Child window 1");
    childWnd.setBackgroundColor({ 100, 100, 100 });

    Win32Window::ChildWindow childWnd2(&window);
    //childWnd2.addCaption();
    childWnd2.setTitle(L"Child window 2");
    childWnd2.setBackgroundColor({ 10, 1, 1 });

    Win32Window::ChildWindow childWnd3(&window);
    // //childWnd2.addCaption();
    childWnd3.setTitle(L"Child window 3");
    childWnd3.setBackgroundColor({ 120, 100, 100 });

    // Win32Window::ChildWindow childWnd4(&window);
    // //childWnd2.addCaption();
    // childWnd4.setTitle(L"Child window 4");
    // childWnd4.setBackgroundColor({ 180, 180, 100 });

    // Win32Window::ChildWindow childWnd5(&window);
    // //childWnd2.addCaption();
    // childWnd5.setTitle(L"Child window 5");
    // childWnd5.setBackgroundColor({ 80, 80, 255 });

    Layout* layout = new VBoxLayout(&childWnd);
    Layout* layout2 = new VBoxLayout(&childWnd2);
    Layout* layout3 = new VBoxLayout(&childWnd3);


    Widgets::Button* button = new Widgets::Button(NbRect<int>(100, 100, 100, 100));
    button->setText(L"Set Mode to points");
    button->setOnClickCallback([&window]()
        {
            g_engine->getRenderer()->togglePolygonVisibilityMode(nb::Renderer::Renderer::PolygonMode::POINTS);
        });

    Widgets::Button* button2 = new Widgets::Button(NbRect<int>(100, 100, 100, 100));
    button2->setText(L"Set Mode to full");
    button2->setOnClickCallback([&window]()
        {
            g_engine->getRenderer()->togglePolygonVisibilityMode(nb::Renderer::Renderer::PolygonMode::FULL);
        });

    button->setDisable();
    button2->setDisable();

    Widgets::Label* lb = new Widgets::Label(NbRect<int>(100, 100, 100, 100));

    Widgets::Button* button3 = new Widgets::Button(NbRect<int>(100, 100, 100, 100));
    button3->setText(L"LEFT");
    button3->setOnClickCallback([&window, &lb]()
        {
            g_engine->invokeAsync([](nb::Core::Engine& e)
                {
                    e.getShaderSystem().reloadAll();
                });
        });

    Widgets::TreeView* treeView = new Widgets::TreeView(NbRect<int>(100, 100, 100, 100));

    Widgets::CheckBox* cb = new Widgets::CheckBox(NbRect<int>(100, 100, 100, 100));
    cb->setText(L"TOGGLE");
    layout3->addWidget(cb);

    Widgets::CheckBox* cb1 = new Widgets::CheckBox(NbRect<int>(100, 100, 100, 100));
    cb1->setText(L"TOGGLE1");
    layout3->addWidget(cb1);

    Widgets::CheckBox* cb2 = new Widgets::CheckBox(NbRect<int>(100, 100, 100, 100));
    cb2->setText(L"TOGGLE2");
    layout3->addWidget(cb2);


    Widgets::ComboBox* comboBox = new Widgets::ComboBox();
    layout3->addWidget(comboBox);

    Widgets::CheckBox* cb3 = new Widgets::CheckBox(NbRect<int>(100, 100, 100, 100));
    cb3->setText(L"TOGGLE3");
    layout3->addWidget(cb3);

    Widgets::ComboBox* comboBox2 = new Widgets::ComboBox();
    layout3->addWidget(comboBox2);

    Widgets::CheckBox* cb4 = new Widgets::CheckBox(NbRect<int>(100, 100, 100, 100));
    cb4->setText(L"TOGGLE4");
    layout3->addWidget(cb4);

    Widgets::Button* button4 = new Widgets::Button(NbRect<int>(100, 100, 100, 100));
    button4->setText(L"LEFT");
    layout3->addWidget(button4);

    lb->setText(L"Hello world");
    lb->setHTextAlign(Widgets::Label::HTextAlign::BOTTOM);
    lb->setVTextAlign(Widgets::Label::VTextAlign::CENTER);

    layout2->addWidget(treeView);
    layout->addWidget(lb);
    layout->addWidget(button);
    layout->addWidget(button2);
    layout->addWidget(button3);
    DockManager dockManager(&window);
    dockManager.addWindow(nullptr, &sceneWindow, DockPlacement::CENTER);
    dockManager.addWindow(nullptr, &childWnd, DockPlacement::LEFT);
    dockManager.addWindow(nullptr, &childWnd2, DockPlacement::RIGHT);
    dockManager.addWindow(&childWnd2, &childWnd3, DockPlacement::BOT);
    //dockManager.addWindow(nullptr, &childWnd5, DockPlacement::TOP);
    //dockManager.addWindow(&childWnd, &childWnd3, DockPlacement::TOP);
    //dockManager.addWindow(nullptr, &childWnd4, DockPlacement::RIGHT);
    // dockManager.addWindow(nullptr, &childWnd2, DockPlacement::RIGHT);

    //dockManager.update(dockManager.getTree()->getRoot());
    // Layout* parent = new VBoxLayout(&window);
    // Layout* sceneLayout = new VBoxLayout(&sceneWindow);
    // Layout *childLayout = new VBoxLayout(&childWnd);

    // parent->addLayout(sceneLayout);
    // parent->addLayout(childLayout);



    //g_engine = std::make_shared<nb::Core::Engine>(sceneWindow.getHandle().as<HWND>());
    window.show();
    childWnd.show();
    childWnd2.show();
    childWnd3.show();

    const NbSize<int>& size = sceneWindow.getSize();

    nb::Core::EngineSettings::setHeight(size.height);
    nb::Core::EngineSettings::setWidth(size.width);
    //childWnd.show();
    g_running = true;
    std::thread engineThread(engineThreadFunc, g_engine.get(), sceneWindow.getHandle().as<HWND>());


    MSG msg;
    bool notInit = false;
    bool running = true;
    while (running)
    {
        if (g_init && !notInit)
        {
            sceneWindow.setRenderable(false);
            button->setDefault();
            button2->setDefault();
            childWnd.repaint();

            auto model = std::make_shared<SceneModel>(g_engine->getRenderer()->getScene());
            treeView->setModel(model);
            notInit = true;
            childWnd2.repaint(); // этому коду срочно нужен рефакторинг)))))) 

            subscribe(*treeView, &Widgets::TreeView::onItemClickSignal, [&treeView](Widgets::ModelIndex index) {
                if (index.isValid())
                {
                    treeView->setItemState(index, Widgets::TreeView::ItemState::COLLAPSED);
                }
            });

            subscribe(*treeView, &Widgets::TreeView::onItemClickSignal, [&treeView, &childWnd3](Widgets::ModelIndex index) {

                childWnd3.setTitle(Utils::toWstring(treeView->getModel()->data(index)));
                childWnd3.repaint();

            });

            subscribe(*cb, &Widgets::CheckBox::onCheckStateChanged, [&cb](bool state) {
                if (state)
                {
                    g_engine->getRenderer()->togglePolygonVisibilityMode(nb::Renderer::Renderer::PolygonMode::POINTS);
                }
                else
                {
                    g_engine->getRenderer()->togglePolygonVisibilityMode(nb::Renderer::Renderer::PolygonMode::FULL);
                }
                });

        }

        g_input = false;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                g_running = false;
                running = false;
                continue;
            }
            if (msg.message == WM_INPUT)
            {
                g_engine->bufferizeInput(msg);
            }

            if (window.isSizeChanged())
            {
                dockManager.onSize(window.getClientRect());
            }

            if (sceneWindow.isSizeChanged())
            {
                const NbSize<int>& size = sceneWindow.getSize();

                nb::Core::EngineSettings::setHeight(size.height);
                nb::Core::EngineSettings::setWidth(size.width);
            }

            if (g_engine)
            {
                if (g_engine->getMode() == nb::Core::Engine::Mode::EDITOR)
                {
                    window.showCursor();
                }
                else
                {
                    window.hideCursor();
                }
            }


            window.resetStateDirtyFlags();
            sceneWindow.resetStateDirtyFlags();

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        g_input = true;
        //g_engine->run({}, {});



        if (!running)
            break;
    }
    engineThread.detach();

    auto i = GetLastError();

    return (int)msg.wParam;
}
