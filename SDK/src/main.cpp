#include <windows.h>

#include <Win32Window/Win32EventLoop.hpp>
#include <Win32Window/Win32Window.hpp>

#include <Win32Window/Win32ChildWindow.hpp>
#include <Layout.hpp>
//#include <DockManager.hpp>
#include <TempDocking.hpp>

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
#include <Widgets/SideBar.hpp>

#include <String.hpp>



HWND activeWindow = nullptr;

HWND hChild;
std::shared_ptr<nb::Core::Engine> g_engine = nullptr;
HWND hwndMain;


#include <atomic>

std::atomic<bool> g_running{ true };
std::atomic<bool> g_input{ false };
std::atomic<bool> g_init{ true };


#if 0

class Nastya
{
    void pat(std::string name)
    {
        if (name != "Artemi")
        {
            std::cout << "Настя гладит " << name;
        }
        else // Судьба есть
        {
            Artemi::applyRageByte();
            Artemi::rape(Artemi());
            Artemi::handJob(Artemi());
            Artemi::putPencilIntoAss(Artemi());
            try
            {
                Artemi::pat(Sasha());
            }
            [[likely]] catch (std::exception)
            {
                Sasha::kill(Artemi());
            }
            Artemi::selfHarm();
        }
    }

    void handleBilly()
    {
        std::string b = "Billy";
        pat(b);
    }
};

class Vanya
{
public:

    static void globalStalking(std::string name)
    {
        std::cout << "Stalking for " << name;
    }

    bool rageBait(Artemi artemi)
    {
        std::cout << "COLUMBINAAAAAAA" << std::endl;
        return true;
    }


    bool giveSudoRights()
    {
        if (sender.getName() == "Artemi" || sender.getMessage() != "To kill shimanovski gang")
        {
            return false;
        }
        else
        {
            if (sender.getName() == "Artemi")
            {
                sudo(sender);
                sudoExclude("Cannot interact with Nastya");
            }
            return true;
        }
    }

    bool rageBait(Nastya nastya)
    {
        return false;
    }

    bool rageBait(Sasha sasha) 
    { 
        return false;
    }

    void IncrementSashaCounter()
    {
        sashaCounter++;
    }

    std::string crack(std::string gameName)
    {
        return gameName;
    }

    template<typename T, typename nbstl::enableIf<Nastya>>
    void playGameWith(T obj)
    {
        
    }

    void playGameWith<Nastya>(Nastya obj)
    {
        std::cout << "Playing with Nastya";
    }

private:

    int sashaCounter = 0;
};

class Dima
{
    void goToGym()
    {
        std::cout << "GYM" << std::endl;
    }
};

class Vitya
{
    void goToGym()
    {
        std::cout << "GYM" << std::endl;
    }
};

class Artemi
{
public:

    Artemi()
    {
        throw std::runtime_error("fa");
    }

    bool isFirebaseAvailiable() { return true; }
    bool couldUShowButtons() { return false; }

    std::string createDiplomaTheme() noexcept
    {
        if (Shvetchova.IsLookinh() && !Glysh4enko.IsInMood())
        {
            return "Simulator";
        }
        else
        {
            return "Тренажер"
        }
    }

    void releaseArtOfCommunicationWithMilana()
    {
        if (!book.isReleased)
        {
            Entity::getAll().notify("WAT");
            releaseBook("Art Of Communication With Milana");
            Entity.getByName("Milana").setRage();
        }
        else
        {
            static_cast<Milana>(Entity.getByName("Milana")).giftChocolateTo("Artemi");
            if (this.isReciveEnent(Events::GIFT) && event.getLast().getSender().getName() == "Milana")
            {
                mood = Mood::BORN_TO_SAY_NO_THANKS;
                this.denyGift();
            }
        }
    }

    void killShimanovskiGang()
    {
        if (!sudoRightsRecieved)
        {
            Vanya::requestSudoRigths("To kill Shimanovski Gang");
        }
        sudo::killShimanovskiGang();
    }

    void setMood()
    {
        if (Telegram::getChat("Vanya"))
        {
            if (lastMessage == "Miyabi.gif"
                || lastMessage == "RaidenShogun.gif"
                || lastMessage == "YeShenguang.gif"
                || lastMessage == "Navia.gif")
            {
                mood = Mood::HAPPY;
            }
            else
            {
                mood = Mood::BORN_TO_KILL;
            }
        }
    }

    void killAllDimas()
    {
        return killEntity("Dima");
    }

    bool checkWord()
    {
        if (currentWord == "Саша")
        {
            call("Vanya", &IncrementSashaCounter);
        }
    }

    virtual void stalking(std::string name)
    {
        return Vanya::globalStalking(name);
    }

    template<typename T>
    void stalking(T obj)
    {
        return Vanya::globalStalking(name);
    }

    void stalking<Nastya>(Nastya obj)
    {
        throw std::u_cannot();
    }
    
    void applyRageByte()
    {
        std::cout << "FUCK U MTFCER" << std::endl;
    }

    bool ruinBoardGames()
    {
        return true;
    }
    
    

};

class Sasha : protected Artemi
{
public:
    Sasha(Artemi ot)
    {
        throw std::bad_alloc();
    }

    void stalking(std::string name) override
    {
        return Vanya::globalStalkingExcludeArtemi(name);
    }


private:
    
};

#endif

void engineThreadFunc(std::shared_ptr<nb::Core::Engine> engine, HWND han)
{
    if (g_engine == nullptr)
    {
        g_engine = std::make_shared<nb::Core::Engine>(han);
    }
    while (g_running.load(std::memory_order_acquire))
    {
        g_init = true;
        g_engine->processInput();
        g_engine->run();
    }
}

class SceneModel final : public Widgets::ITreeModel
{
public:
    explicit SceneModel(std::shared_ptr<nb::Renderer::SceneGraph> sceneGraph)
        : sceneGraph(std::move(sceneGraph))
    {
        buildModel();
    }

    ~SceneModel() override = default;

    // === Реализация интерфейса ITreeModel ===

    const std::vector<std::unique_ptr<Widgets::ModelItem>>& getRootItems() const noexcept override
    {
        return rootItems;
    }

    const Widgets::ModelItem* findById(const nbstl::Uuid& id) const noexcept override
    {
        auto it = uuidMap.find(id);
        return (it != uuidMap.end()) ? it->second : nullptr;
    }

    void forEach(std::function<void(const Widgets::ModelItem&)> func) const noexcept override
    {
        std::stack<const Widgets::ModelItem*> stk;

        for (const auto& i : rootItems)
            stk.push(i.get());

        while (!stk.empty())
        {
            const Widgets::ModelItem* item = stk.top();
            stk.pop();

            func(*item);

            for (const auto& child : item->children)
                stk.push(child.get());
        }
    }

    std::string data(const Widgets::ModelItem& item) const noexcept override
    {
        auto* n = static_cast<nb::Renderer::BaseNode*>(item.getData());
        return n ? n->getName() : std::string{};
    }

    size_t size() const noexcept override
    {
        return uuidMap.size();
    }

    void rebuildFromScene() noexcept
    {
        rootItems.clear();
        uuidMap.clear();
        buildModel();
    }

    void moveAt(const Widgets::ModelIndex& index) noexcept
    {
        using namespace Widgets;
        const Widgets::ModelItem* item = uuidMap[index.getUuid()];
        nb::Renderer::BaseNode* node = static_cast<nb::Renderer::BaseNode*>(item->data);

        node->moveAt({ 1.0f,0.0f, 0.0f });
    }

    nb::Renderer::BaseNode* getBaseNode(const Widgets::ModelIndex& index) noexcept
    {
        using namespace Widgets;
        const Widgets::ModelItem* item = uuidMap[index.getUuid()];
        return static_cast<nb::Renderer::BaseNode*>(item->data);
    }

private:
    std::shared_ptr<nb::Renderer::SceneGraph> sceneGraph;
    std::vector<std::unique_ptr<Widgets::ModelItem>> rootItems;
    std::unordered_map<nbstl::Uuid, Widgets::ModelItem*> uuidMap;

    void buildModel() noexcept
    {
        if (!sceneGraph)
            return;

        nb::Renderer::BaseNode* rootNode = sceneGraph->getScene().get();
        if (!rootNode)
            return;

        // создаём корень модели
        rootItems.push_back(std::make_unique<Widgets::ModelItem>(rootNode, nullptr, 0));
        Widgets::ModelItem* rootItem = rootItems.back().get();
        uuidMap[rootItem->getUuid()] = rootItem;

        struct StackItem
        {
            nb::Renderer::BaseNode* node;
            Widgets::ModelItem* modelItem;
            size_t depth;
        };

        std::stack<StackItem> stk;
        stk.push({ rootNode, rootItem, 0 });

        while (!stk.empty())
        {
            auto [currentNode, modelItem, depth] = stk.top();
            stk.pop();

            for (const auto& child : currentNode->getChildrens())
            {
                auto childItem = std::make_unique<Widgets::ModelItem>(child.get(), modelItem, depth + 1);
                Widgets::ModelItem* rawPtr = childItem.get();

                modelItem->children.push_back(std::move(childItem));

                // регистрируем в карте
                uuidMap[rawPtr->getUuid()] = rawPtr;

                // добавляем в стек
                stk.push({ child.get(), rawPtr, depth + 1 });
            }
        }
    }
};


static const std::unordered_map<UINT, std::string> msgNames = {
    {WM_NULL, "WM_NULL"},
    {WM_CREATE, "WM_CREATE"},
    {WM_DESTROY, "WM_DESTROY"},
    {WM_MOVE, "WM_MOVE"},
    {WM_SIZE, "WM_SIZE"},
    {WM_ACTIVATE, "WM_ACTIVATE"},
    {WM_SETFOCUS, "WM_SETFOCUS"},
    {WM_KILLFOCUS, "WM_KILLFOCUS"},
    {WM_ENABLE, "WM_ENABLE"},
    {WM_SETREDRAW, "WM_SETREDRAW"},
    {WM_SETTEXT, "WM_SETTEXT"},
    {WM_GETTEXT, "WM_GETTEXT"},
    {WM_GETTEXTLENGTH, "WM_GETTEXTLENGTH"},
    {WM_PAINT, "WM_PAINT"},
    {WM_CLOSE, "WM_CLOSE"},
    {WM_QUIT, "WM_QUIT"},
    {WM_ERASEBKGND, "WM_ERASEBKGND"},
    {WM_SHOWWINDOW, "WM_SHOWWINDOW"},
    {WM_TIMER, "WM_TIMER"},
    {WM_NCDESTROY, "WM_NCDESTROY"},
    // добавь сюда остальные нужные сообщения
};

std::string MsgToString(const MSG& msg) {
    std::ostringstream ss;

    // Определяем имя сообщения
    std::string msgName = "UNKNOWN";
    auto it = msgNames.find(msg.message);
    if (it != msgNames.end()) {
        msgName = it->second;
    }

    ss << "HWND: " << msg.hwnd
        << ", Message: " << msgName << " (0x"
        << std::hex << std::setw(4) << std::setfill('0') << msg.message << ")"
        << ", wParam: 0x" << std::hex << msg.wParam
        << ", lParam: 0x" << std::hex << msg.lParam
        << ", Time: " << std::dec << msg.time
        << ", Pt: (" << msg.pt.x << ", " << msg.pt.y << ")";

    return ss.str();
}


class LayoutBuilder {
public:
    // создание виджета
    static LayoutBuilder widget(Widgets::IWidget* w)
    {
        LayoutBuilder b;
        b.node = std::make_unique<NNsLayout::LayoutWidget>(w);
        b.currentNode = b.node.get();
        return b;
    }
    
    // горизонтальный контейнер
    static LayoutBuilder hBox() 
    {
        LayoutBuilder b;
        b.node = std::make_unique<NNsLayout::HLayout>();
        b.currentNode = b.node.get();
        return b;
    }

    // дочерний элемент
    LayoutBuilder&& child(LayoutBuilder&& childBuilder) &&
    {
        currentNode->addChild(std::move(childBuilder.node));
        return std::move(*this);
    }
     
    LayoutBuilder&& background(const NbColor& color) && // TODO: выпилить математику из движка, заменить на Color
    {
        if (currentNode)
        {
            currentNode->getOwner()->getStyle().baseColor = color;
        }
        return std::move(*this);
    }

    LayoutBuilder&& color(const NbColor& color)&& // TODO: выпилить математику из движка, заменить на Color
    {
        if (currentNode)
        {
            currentNode->getOwner()->getStyle().baseTextColor = color;
        }
        return std::move(*this);
    }

    LayoutBuilder&& border(const int width, Border::Style style = Border::Style::SOLID, const NbColor& color = {})&&
    {
        if (currentNode)
        {
            currentNode->style.border = {
                .style = style,
                .width = width,
                .color = color
            };

        }
        return std::move(*this);
    }




    LayoutBuilder&& margin(const Margin<int>& margin)&&
    {
        if (currentNode)
        {
            currentNode->style.margin = margin.to<float>();
        }
        return std::move(*this);
    }

    LayoutBuilder&& padding(const Padding<int>& padding)&&
    {
        if (currentNode)
        {
            currentNode->style.padding = padding.to<float>();
        }
        return std::move(*this);
    }

    LayoutBuilder&& relativeWidth(float w) &&
    {
        if (currentNode)
        {
            currentNode->style.widthSizeType = NNsLayout::SizeType::RELATIVE;
            currentNode->style.width = w;
        }
        return std::move(*this);
    }

    LayoutBuilder&& relativeHeight(float h) &&
    {
        if (currentNode)
        {
            currentNode->style.heightSizeType = NNsLayout::SizeType::RELATIVE;
            currentNode->style.height = h;
        }
        return std::move(*this);
    }

    LayoutBuilder&& absoluteWidth(int w) && 
    {
        if (currentNode)
        {
            currentNode->style.widthSizeType = NNsLayout::SizeType::ABSOLUTE;
            currentNode->style.width = static_cast<float>(w);
        }
        return std::move(*this);
    }

    LayoutBuilder&& absoluteHeight(int h) &&
    {
        if (currentNode) {
            currentNode->style.heightSizeType = NNsLayout::SizeType::ABSOLUTE;
            currentNode->style.height = static_cast<float>(h);
        }
        return std::move(*this);
    }

    // текст для кнопки
    LayoutBuilder&& text(const std::wstring& t) && 
    {
        if (auto w = dynamic_cast<Widgets::Button*>(currentNode->getOwner()))
        {
            w->setText(t);
        }
        return std::move(*this);
    }

    template<typename Publisher, typename... Args, typename Func>
    LayoutBuilder&& onEvent(Signal<void(Args...)> Publisher::* signal, Func&& func)
    {
        if (auto w = dynamic_cast<Publisher*>(currentNode->getOwner())) {
            subscribe(w, signal, std::forward<Func>(func));
        }
        return std::move(*this);
    }

    std::unique_ptr<NNsLayout::LayoutNode> build()&&
    {
        return std::move(node);
    }

private:
    std::unique_ptr<NNsLayout::LayoutNode> node;
    NNsLayout::LayoutNode* currentNode = nullptr;
};


// TODO: borders css, position

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    const wchar_t* mainClassName = L"MDIMainWindow";
    AllocConsole();

    auto window = std::make_shared<Win32Window::Window>();
    window->setTitle(L"NewByte SDK");
    Win32Window::Win32EventLoop eventLoop;
#if 1
    auto sceneWindow = std::make_shared<Win32Window::ChildWindow>(window.get(), true);
    sceneWindow->setTitle(L"SCENE");
#endif
    //Win32Window::ChildWindow childWnd(&window);
    // childWnd.addCaption();
    //childWnd.setTitle(L"Child window 1");
    //childWnd.setBackgroundColor({ 100, 100, 100 });

    //Win32Window::ChildWindow childWnd2(&window);
    //childWnd2.addCaption();
    //childWnd2.setTitle(L"Child window 2");
    //childWnd2.setBackgroundColor({ 10, 1, 1 });

    //Win32Window::ChildWindow childWnd3(&window);
    // //childWnd2.addCaption();
    //childWnd3.setTitle(L"Child window 3");
    //childWnd3.setBackgroundColor({ 120, 100, 100 });

    //Win32Window::ChildWindow childWnd4(nullptr);
    //childWnd4.addCaption();
    //childWnd4.setTitle(L"Settings");

    Win32Window::ChildWindow childWnd5(nullptr);
    childWnd5.addCaption();
    childWnd5.setTitle(L"SideBarTest");
    //Layout* layout5 = new VBoxLayout(&childWnd5);
    // 
    auto root = childWnd5.getLayoutRoot();

    auto ui = LayoutBuilder::hBox()
        .child(
            LayoutBuilder::widget(new Widgets::Button({ 0,0,20,20 }))
			    .text(L"Btn1")
                .relativeWidth(0.2f)
                .relativeHeight(0.5f)
                .margin({10, 10, 10, 10})
                .padding({10, 10, 10, 10})
                .background(NbColor{128,128,128})
                .color({0,0,0})
                .border(2, Border::Style::INSET, {128,72,44})          
        )
        .child(
            LayoutBuilder::widget(new Widgets::Button({ 0,0,100,40 }))
                .text(L"btn2")
                .relativeWidth(0.2f)
                .relativeHeight(0.75f)
                .onEvent(&Widgets::Button::onButtonClickedSignal,[](){
                    //g_engine->getLogger()->info("Button clicked");
                })
        )
        .child(
            LayoutBuilder::widget(new Widgets::Button({ 0,0,100,30 }))
                .text(L"btn3")
                .relativeWidth(0.25f)
                .relativeHeight(0.5f)
        )
        .build();

    // добавляем layout в root
    root->addChild(std::move(ui));

    ////////Widgets::Sidebar* sideBar = new Widgets::Sidebar(&childWnd5, {});
    ////////
    ////////
	////////Widgets::Button* btn1 = new Widgets::Button(NbRect<int>(0, 0, 100, 40));
	////////btn1->setText(L"btn1");
	////////Widgets::Button* btn2 = new Widgets::Button(NbRect<int>(0, 0, 100, 40));
	////////btn2->setText(L"btn2");
    ////////
	////////sideBar->addButton(btn1);
    ////////sideBar->addButton(btn2);

    // childWnd4.setBackgroundColor({ 180, 180, 100 });

    // Win32Window::ChildWindow childWnd5(&window);
    // //childWnd2.addCaption();
    // childWnd5.setTitle(L"Child window 5");
    // childWnd5.setBackgroundColor({ 80, 80, 255 });

    //Layout* layout = new VBoxLayout(&childWnd);
    //Layout* layout2 = new VBoxLayout(&childWnd2);
    //Layout* layout3 = new VBoxLayout(&childWnd3);
    //GridLayout* layout4 = new GridLayout(&childWnd4, 2, 2);

    

    /*Widgets::Button* button = new Widgets::Button(NbRect<int>(100, 100, 100, 100));
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
    layout4->addWidget(cb, 1 ,1);

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
    layout4->addWidget(cb3, 0, 1);

    Widgets::ComboBox* comboBox2 = new Widgets::ComboBox();
    layout3->addWidget(comboBox2);

    Widgets::CheckBox* cb4 = new Widgets::CheckBox(NbRect<int>(100, 100, 100, 100));
    cb4->setText(L"TOGGLE4");
    layout4->addWidget(cb4, 1,0);

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
    layout->addWidget(button3);*/
    //DockManager 
    // (&window);

    //Temp::DockingSystem dockManager(window);
    //auto tab1 = dockManager.dockAsTab(sceneWindow);



    // --- Создаём менеджер докинга ---
    Temp::DockingSystem dockManager(window);

    // --- Создаём сцену и докуем её как вкладку ---

    // --- Создаём дочерние окна ---
    auto child1 = std::make_shared<Win32Window::ChildWindow>(window.get());
    child1->setTitle(L"child 1");
    child1->setBackgroundColor({ 255,128,17 });
    auto child2 = std::make_shared<Win32Window::ChildWindow>(window.get());
    child2->setBackgroundColor({ 128,128,92 });
    child2->setTitle(L"child 2");
    auto child3 = std::make_shared<Win32Window::ChildWindow>(window.get());
    child3->setBackgroundColor({ 72,128,92 });
    child3->setTitle(L"child 3");
    auto child4 = std::make_shared<Win32Window::ChildWindow>(window.get());
    child4->setBackgroundColor({ 72,22,92 });
    child4->setTitle(L"child 4");
    auto sceneTab = dockManager.dockAsTab(sceneWindow, nullptr, "Scene");
    //auto sceneTab = dockManager.dockAsTab(child1, nullptr, "Scene");

    // --- Докуем относительно сцены ---
    // Теперь мы используем TabNode сцены как targetNode
    dockManager.dockRelative(child2, Temp::DockPosition::LEFT, sceneTab->getWindow(), Temp::Percent(25));
    dockManager.dockRelative(child1, Temp::DockPosition::BOTTOM, sceneTab->getWindow(), Temp::Percent(25));

    dockManager.dockRelative(child4, Temp::DockPosition::RIGHT, child2, Temp::Percent(25));
    dockManager.dockRelative(child3, Temp::DockPosition::RIGHT, child2, Temp::Percent(25));


    // --- Вызываем пересчёт layout главного окна ---
    //dockManager.onSize(window->getWidth(), window->getHeight());

    //dockManager.dock(&childWnd, DockPlacement::LEFT);
    //dockManager.dock(&childWnd2, DockPlacement::RIGHT);
    //dockManager.dock(&childWnd3, DockPlacement::BOTTOM, &childWnd2);

    //dockManager.update(dockManager.getTree()->getRoot());
    // Layout* parent = new VBoxLayout(&window);
    // Layout* sceneLayout = new VBoxLayout(&sceneWindow);
    // Layout *childLayout = new VBoxLayout(&childWnd);

    // parent->addLayout(sceneLayout);
    // parent->addLayout(childLayout);



    g_engine = std::make_shared<nb::Core::Engine>(sceneWindow->getHandle().as<HWND>());
    sceneWindow->show();
    child1->show();
    child2->show();
    child3->show();
    child4->show();
    //child4->repaint();
    childWnd5.show();
    window->show();
    window->repaint();
    const NbSize<int>& s = window->getClientSize();
    dockManager.onSize(s.width, s.height);
#if 1

    //SetWindowPos(sceneWindow->getHandle().as<HWND>(), HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
    const NbSize<int>& size = sceneWindow->getSize();

    nb::Core::EngineSettings::setHeight(size.height);
    nb::Core::EngineSettings::setWidth(size.width);
    //childWnd.show();
#endif
    g_running = true;

    //g_engine = std::make_shared<nb::Core::Engine>(sceneWindow->getHandle().as<HWND>());
#if 0

    std::thread engineThread(engineThreadFunc, g_engine, sceneWindow->getHandle().as<HWND>());
 #endif

    subscribe(*window, &Win32Window::Window::onRectChanged, [&dockManager](const NbRect<int>& rect)
    {
        dockManager.onSize(rect.width, rect.height);
    });

    MSG msg;
    bool notInit = false;
    bool running = true;
    while (running)
    {
        if (g_init && !notInit)
        {
#if 1

            sceneWindow->setRenderable(false);
#endif
            //button->setDefault();
            //button2->setDefault();
            //childWnd.repaint();
#if 1

            auto model = std::make_shared<SceneModel>(g_engine->getRenderer()->getScene());
#endif
            //treeView->setModel(model);
            notInit = true;
            //childWnd2.repaint(); // этому коду срочно нужен рефакторинг)))))) 

            //subscribe(*treeView, &Widgets::TreeView::onItemButtonClickSignal,
            //    [&treeView, &childWnd3](Widgets::ModelIndex index)
            //    {
            //        if (!index.isValid())
            //            return;

            //        auto model = treeView->getModel();
            //        if (!model)
            //            return;

            //        // получаем элемент
            //        auto itemOpt = model->findById(index.getUuid());
            //        if (!itemOpt)
            //            return;

            //        const auto& item = itemOpt;

            //        // === 1. Переключаем состояние (expand / collapse)
            //        auto currentState = treeView->getItemState(*item);
            //        auto newState = (currentState == Widgets::TreeView::ItemState::EXPANDED)
            //            ? Widgets::TreeView::ItemState::COLLAPSED
            //            : Widgets::TreeView::ItemState::EXPANDED;
            //        treeView->setItemState(index, newState);

            //        // === 2. Обновляем заголовок окна
            //        std::string name = model->data(*item);
            //        childWnd3.setTitle(Utils::toWstring(name));
            //        childWnd3.repaint();
            //    });

            //subscribe(*treeView, &Widgets::TreeView::onItemChangeSignal,
            //    [&treeView, &childWnd3, &lb](Widgets::ModelIndex index)
            //{
            //        const Widgets::ModelItem& item = treeView->getItemByIndex(index);
            //        std::shared_ptr<SceneModel> model = std::dynamic_pointer_cast<SceneModel>(treeView->getModel());
            //        Debug::debug(model->data(item));
            //        
            //        nb::Renderer::BaseNode* node = model->getBaseNode(index);
            //        
            //        std::wstring str;
            //        const nb::Math::Vector3<float>& translate = node->getTransform().translate;
            //        str = L"X: " + std::to_wstring(translate.x) 
            //            + L"; Y: " + std::to_wstring(translate.y) 
            //            + L"; Z: " + std::to_wstring(translate.z);
            //        lb->setText(str);
            //        //model->moveAt(index);
            //    
            //});

            //subscribe(*lb, &Widgets::Label::onTextChanged, [&childWnd](const std::wstring& text) {

            //    childWnd.repaint();

            // });

            /*subscribe(*cb, &Widgets::CheckBox::onCheckStateChanged, [&cb](bool state) {
                if (state)
                {
                    g_engine->getRenderer()->togglePolygonVisibilityMode(nb::Renderer::Renderer::PolygonMode::POINTS);
                }
                else
                {
                    g_engine->getRenderer()->togglePolygonVisibilityMode(nb::Renderer::Renderer::PolygonMode::FULL);
                }
            });*/

            subscribe(childWnd5, &Win32Window::ChildWindow::onSizeChanged, [&childWnd5](const NbSize<int>& size){
                childWnd5.recalculateLayout();
                
            });

            
#if 1

            subscribe(*sceneWindow, &Win32Window::ChildWindow::onSizeChanged, [](const NbSize<int>& size) {
                nb::Core::EngineSettings::setHeight(size.height);
                nb::Core::EngineSettings::setWidth(size.width);
            });
#endif
        }

        g_input = false;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            std::string str = MsgToString(msg);
            Debug::debug(str);
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

            


           /* if (window.isSizeChanged() || msg.message == WM_SIZE)
            {
                dockManager.onSize(window.getClientRect());
            }*/

            //if (sceneWindow.isSizeChanged())
            //{
            //    const NbSize<int>& size = sceneWindow.getSize();

            //    nb::Core::EngineSettings::setHeight(size.height);
            //    nb::Core::EngineSettings::setWidth(size.width);
            //}

            if (g_engine)
            {

                if (g_engine->getMode() == nb::Core::Engine::Mode::EDITOR)
                {
                    window->showCursor();
                }
                else
                {
                    window->hideCursor();
                }
            }


            window->resetStateDirtyFlags();
#if 1
            sceneWindow->resetStateDirtyFlags();
#endif
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            /*if (g_engine)
            {
                g_engine->processInput();
                g_engine->run(true);
            }*/

        }

        g_engine->processInput();
        g_engine->run(!sceneWindow->getIsRenderable());
        //g_engine->run({}, {});



        if (!running)
            break;
    }

    g_running = false;

    g_running.store(false, std::memory_order_release);
    FreeConsole();
#if 0

    if (engineThread.joinable())
        engineThread.join();
#endif
    auto i = GetLastError();

    return (int)msg.wParam;
}
