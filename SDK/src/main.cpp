#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <windows.h>

//#include "MemoryTracker.hpp"
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
#include <Widgets/SpinBox.hpp>

#include <String.hpp>

#include <Error/ErrorManager.hpp>
#include <Error/ErrorConsolePrinter.hpp>


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


class Spacer : public Widgets::IWidget
{
public:
    // Inherited via IWidget
    bool hitTest(const NbPoint<int>& pos) override
    {
        return false;
    }
    const char* getClassName() const override
    {
        return "SPACER";
    }

    Spacer() noexcept : IWidget({}) {};
    ~Spacer()  {};

    const NbSize<int>& measure(const NbSize<int>& max) noexcept override
    {
        size = { 0, 0 };
        return size;
    }

    void layout(const NbRect<int>& rect) noexcept override
    {
        this->rect = rect;
    }

    NbSize<int> size;

};

class LayoutBuilder 
{
public:
    // создание виджета
    static LayoutBuilder widget(Widgets::IWidget* w)
    {
        LayoutBuilder b;
        b.node = std::make_unique<NNsLayout::LayoutWidget>(w);
        b.currentNode = b.node.get();
        return b;
    }

    static LayoutBuilder label(const std::wstring& text)
    {
        LayoutBuilder b;
        b.node = std::make_unique<NNsLayout::LayoutWidget>(new Widgets::Label(text));
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

    static LayoutBuilder vBox()
    {
        LayoutBuilder b;
        b.node = std::make_unique<NNsLayout::VLayout>();
        b.currentNode = b.node.get();
        return b;
    }

    static LayoutBuilder spacer()
    {
        LayoutBuilder b;
        b.node = std::make_unique<NNsLayout::LayoutWidget>(new Spacer());
        b.currentNode = b.node.get();
        b.currentNode->style.widthSizeType = NNsLayout::SizeType::RELATIVE;
        b.currentNode->style.width = 1.0f;
        b.currentNode->style.heightSizeType = NNsLayout::SizeType::RELATIVE;
        b.currentNode->style.height = 1.0f;
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

    LayoutBuilder&& checked(bool state)&& // TODO: выпилить математику из движка, заменить на Color
    {
        Debug::debug("Checked not impl");
        return std::move(*this);
    }

    LayoutBuilder&& fontSize(int size)&& // TODO: выпилить математику из движка, заменить на Color
    {
        Debug::debug("Font size not impl");
        return std::move(*this);
    }

    LayoutBuilder&& textAlign(Widgets::TextAlign align) && // TODO: выпилить математику из движка, заменить на Color
    {
        Debug::debug("TextAlign not impl");
        return std::move(*this);
    }
    

    LayoutBuilder&& border(const int width, Border::Style style = Border::Style::SOLID, const NbColor& color = {}) &&
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
        else if (auto w = dynamic_cast<Widgets::Label*>(currentNode->getOwner()))
        {
            w->setText(t);
        }
        else if (auto w = dynamic_cast<Widgets::CheckBox*>(currentNode->getOwner()))
        {
            w->setText(t);
        }
        return std::move(*this);
    }

    LayoutBuilder&& setTreeModel(std::shared_ptr<SceneModel>& model) &&
    {
        if (auto w = dynamic_cast<Widgets::TreeView*>(currentNode->getOwner()))
        {
            w->setModel(model);
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

    LayoutBuilder&& style(std::function<void(NNsLayout::LayoutStyle&)> f)&&
    {
        if (currentNode)
        {
            f(currentNode->style);
        }
        return std::move(*this);
    }

    template<typename T, typename Func>
    LayoutBuilder&& apply(Func&& func)&& 
    {
        if (auto w = dynamic_cast<T*>(currentNode->getOwner()))
        {
            func(w);
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


#include "Localization/LocaleManager.hpp"
#include "Localization/LocaleLoader.hpp"
#include "Localization/Formatter.hpp"
#include "Localization/Translation.hpp"
#include "Localization/Calendars/GregorianCalendar.hpp"
#include "Localization/Calendars/JulianCalendar.hpp"
#include "Localization/Calendars/HijriCalendar.hpp"
#include "Localization/Calendars/HebrewCalendar.hpp"

#include "Localization/Calendars/EthiopicCalendar.hpp"
#include "Localization/Calendars/PersianCalendar.hpp"
#include "Widgets/Calendar.hpp"
#include "Localization/PluralEngine.hpp"

void setAppLocale() noexcept
{
    using namespace Localization;
    Locale locale = LocalLoader::load("/Assets/Locales/ru-RU.locale");
    LocaleManager::setCurrent(locale);
    Localization::Translation::load("Assets/Localization/ru.translation");
}


nb::Renderer::BaseNode* activeSceneGraphNode = nullptr;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    const wchar_t* mainClassName = L"MDIMainWindow";
    AllocConsole();

    nb::Error::ErrorManager::instance().setPrinter(new nb::Error::ErrorConsolePrinter());
    nb::Error::ErrorManager::instance().report(nb::Error::Type::FATAL, "DDDD");
    auto window = std::make_shared<Win32Window::Window>();

    setAppLocale();
    auto buffer = Localization::Formatter::toDate(time(nullptr));
    //auto buffer = Localization::Translation::fromKey("Ui.Title");

    std::wstring utf16;
    utf16.resize(buffer.size());
    int len = MultiByteToWideChar(CP_UTF8, 0, buffer.data(), (int)buffer.size(), utf16.data(), (int)utf16.size());
    utf16.resize(len);


    window->setTitle(utf16);
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

    std::shared_ptr<Win32Window::ChildWindow> childWnd5 = std::make_shared<Win32Window::ChildWindow>(nullptr, true);
    childWnd5->addCaption();
    childWnd5->setTitle(L"SideBarTest");
    //Layout* layout5 = new VBoxLayout(&childWnd5);
    // 
    bool shouldShowDebug = false;
    //
    //childWnd5
    //auto root = childWnd5.getLayoutRoot();

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

                        .child(LayoutBuilder::widget(new Widgets::SpinBox())
                            .relativeWidth(0.7f)
                            .absoluteHeight(35)
                            .color(NbColor{ 200, 200, 200 })
                            .textAlign(Widgets::TextAlign::LEFT)
                            .padding({ 0, 0, 0, 0 })
                        )

                        .child(LayoutBuilder::widget(new Widgets::SpinBox())
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


    Win32Window::ChildWindow childWnd6(nullptr);
    childWnd6.addCaption();
    childWnd6.setTitle(L"ButtonStyleTest");


    auto root1 = childWnd6.getLayoutRoot();

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
            .onEvent(&Widgets::CheckBox::onCheckStateChanged, [](bool checked) {
                //g_engine->getRenderer()->setWireframeMode(checked);
             }))

        .child(LayoutBuilder::widget(new Widgets::CheckBox())
            .text(L"Show Grid")
            .relativeWidth(1.0f).absoluteHeight(30)
            .onEvent(&Widgets::CheckBox::onCheckStateChanged, [](bool checked) {
                //g_engine->getRenderer()->setGridVisible(checked);
                }))

        .child(LayoutBuilder::widget(new Widgets::CheckBox())
            .text(L"Display Normals")
            .relativeWidth(1.0f).absoluteHeight(30)
            .onEvent(&Widgets::CheckBox::onCheckStateChanged, [](bool checked) {
                g_engine->getRenderer()->toggleDebugPass();
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
            .apply<Widgets::ComboBox>([](Widgets::ComboBox* c) {
                c->addItem({L"FULL", 1});
                c->addItem({ L"LINES", 2 });
                c->addItem({ L"POINTS", 3 });

            })
            .text(L"Show Draw Calls")
            .relativeWidth(1.0f).absoluteHeight(30)
            .onEvent(&Widgets::CheckBox::onCheckStateChanged, [](bool checked) {
                //g_engine->getRenderer()->toggleStatsOverlay(checked);
                }))
        //.child(LayoutBuilder::widget(new Widgets::ComboBox())
        //    .text(L"Show Draw Calls")
        //    .relativeWidth(1.0f).absoluteHeight(30)
        //    .onEvent(&Widgets::CheckBox::onCheckStateChanged, [](bool checked) {
        //        //g_engine->getRenderer()->toggleStatsOverlay(checked);
        //        }))
        .child(LayoutBuilder::widget(new Widgets::CheckBox())
            .text(L"Show FPS Counter")
            .relativeWidth(1.0f).absoluteHeight(30)
            .onEvent(&Widgets::CheckBox::onCheckStateChanged, [](bool checked) {
                //g_engine->getUI()->setOverlayVisible(L"FPS", checked);
                }))

        .child(LayoutBuilder::spacer()) // Пружина, чтобы все прижалось к верху
        .build();

    root1->addChild(std::move(debugUI));


   

    //auto ui = LayoutBuilder::hBox() // главный горизонтальный контейнер
    //    .style([](NNsLayout::LayoutStyle& s) {
    //    s.widthSizeType = NNsLayout::SizeType::FLEX;
    //    s.heightSizeType = NNsLayout::SizeType::FLEX;
    //    s.padding = { 0,0,0,0 };
    //    s.color= NbColor{ 30,30,30 };
    //        })

    //    // Сайдбар слева
    //    .child(
    //        LayoutBuilder::vBox()
    //        .style([](NNsLayout::LayoutStyle& s) {
    //            s.width = 200; // фиксированная ширина
    //            s.heightSizeType = NNsLayout::SizeType::FLEX;
    //            s.padding = { 5,5,5,5 };
    //            s.color = NbColor{ 50,50,50 };
    //            })
    //        .child(LayoutBuilder::widget(new Widgets::Button({ 0,0,0,0 })).text(L"Category 1").absoluteHeight(40).margin({ 5,5,5,5 }))
    //        .child(LayoutBuilder::widget(new Widgets::Button({ 0,0,0,0 })).text(L"Category 2").absoluteHeight(40).margin({ 5,5,5,5 }))
    //        .child(LayoutBuilder::widget(new Widgets::Button({ 0,0,0,0 })).text(L"Category 3").absoluteHeight(40).margin({ 5,5,5,5 }))
    //    )

    //    // Панель контента справа
    //    .child(
    //        LayoutBuilder::vBox()
    //        .style([](NNsLayout::LayoutStyle& s) {
    //            s.widthSizeType = NNsLayout::SizeType::FLEX; // занимает всё оставшееся пространство
    //            s.heightSizeType = NNsLayout::SizeType::FLEX;
    //            s.padding = { 10,10,10,10 };
    //            s.color= NbColor{ 70,70,70 };
    //            })
    //        .child(
    //            LayoutBuilder::widget(new Widgets::Button({ 0,0,0,0 }))
    //            .text(L"Content for Category 1")
    //            .absoluteHeight(50)
    //            .margin({ 5,5,5,5 })
    //        )
    //        .child(
    //            LayoutBuilder::widget(new Widgets::Button({ 0,0,0,0 }))
    //            .text(L"Content for Category 2")
    //            .absoluteHeight(50)
    //            .margin({ 5,5,5,5 })
    //        )
    //        .child(
    //            LayoutBuilder::widget(new Widgets::Button({ 0,0,0,0 }))
    //            .text(L"Content for Category 3")
    //            .absoluteHeight(50)
    //            .margin({ 5,5,5,5 })
    //        )
    //    )
    //    .build();


    // добавляем layout в root
    //root->addChild(std::move(ui));

    //root

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
//    auto child4 = std::make_shared<Win32Window::ChildWindow>(window.get());
//    child4->setBackgroundColor({ 72,22,92 });
//    child4->setTitle(L"child 4");
    auto sceneTab = dockManager.dockAsTab(sceneWindow, nullptr, "Scene");
    //auto sceneTab = dockManager.dockAsTab(child1, nullptr, "Scene");

    // --- Докуем относительно сцены ---
    // Теперь мы используем TabNode сцены как targetNode
    dockManager.dockRelative(child1, Temp::DockPosition::LEFT, sceneTab->getWindow(), Temp::Percent(25));
    //dockManager.dockRelative(childWnd5, Temp::DockPosition::BOTTOM, child1, Temp::Percent(25));
    dockManager.dockRelative(child2, Temp::DockPosition::BOTTOM, sceneTab->getWindow(), Temp::Percent(25));

    //dockManager.dockRelative(childWnd5, Temp::DockPosition::RIGHT, sceneTab->getWindow(), Temp::Percent(25));
    dockManager.dockRelative(child3, Temp::DockPosition::RIGHT, child2, Temp::Percent(25));

    auto inspector = LayoutBuilder::vBox()
        .style([](NNsLayout::LayoutStyle& s) {
        s.widthSizeType = NNsLayout::SizeType::RELATIVE;
        s.width = 1.0f; // Занимает всю ширину правой панели
        s.padding = { 0, 0, 0, 0 };
        s.color = NbColor{ 35, 35, 35 }; // Цвет фона инспектора
            })

        // Заголовок компонента (Transform)
        .child(LayoutBuilder::label(L"TRANSFORM")
            .relativeWidth(1.0f)
            .absoluteHeight(30)
            .background(NbColor{ 60, 60, 60 })
            .color(NbColor{ 220, 220, 220 })
            .fontSize(14)
            .textAlign(Widgets::TextAlign::LEFT)
            .padding({ 0, 0, 0, 0 }))

        // Строка "Position"
        .child(
            LayoutBuilder::hBox()
            .style([](NNsLayout::LayoutStyle& s) {
                s.heightSizeType = NNsLayout::SizeType::ABSOLUTE;
                s.height = 30;
                s.margin = { 0, 0, 5, 0 }; // Отступ снизу
                s.padding = { 5, 0, 5, 0 };
                })

            // Название свойства
            .child(LayoutBuilder::label(L"Position")
                .relativeWidth(0.35f) // Занимает 35% ширины
                .color(NbColor{ 180, 180, 180 })
                .textAlign(Widgets::TextAlign::LEFT)
                )

            // Контейнер для X, Y, Z
            .child(
                LayoutBuilder::hBox()
                .style([](NNsLayout::LayoutStyle& s) {
                    s.widthSizeType = NNsLayout::SizeType::RELATIVE;
                    s.width= 0.65f; 
                    //s.spacing = 4; // Расстояние между X, Y, Z (если поддерживается)
                    })

                // Поле X
                .child(LayoutBuilder::vBox().relativeWidth(0.33f)
                    .child(LayoutBuilder::widget(new Widgets::SpinBox())
                        .apply<Widgets::SpinBox>([](Widgets::SpinBox* c) {
                            c->setRange(-100, 100);
                        })
                        .relativeWidth(1.0f)
                        .absoluteHeight(30)
                        .background(NbColor{ 25, 25, 25 })
                        .color(NbColor{ 255, 100, 100 })
                        .onEvent(&Widgets::SpinBox::onValueChangedByStep, [](int value){
                            nb::Error::ErrorManager::instance().report(nb::Error::Type::INFO, std::to_string(value));
                            
                            if (!activeSceneGraphNode)
                            {
                                return;
                            }

                            activeSceneGraphNode->addTranslate({ (float)value, 0.0f, 0.0f });
                        })
                    )
                )

                // Поле Y
                .child(LayoutBuilder::vBox().relativeWidth(0.33f)
                    .child(LayoutBuilder::widget(new Widgets::SpinBox())
                        .apply<Widgets::SpinBox>([](Widgets::SpinBox* c) {
                            c->setRange(-100, 100);
                        })
                        .relativeWidth(1.0f)
                        .absoluteHeight(30)
                        .background(NbColor{ 25, 25, 25 })
                        .color(NbColor{ 100, 255, 100 })
                        .onEvent(&Widgets::SpinBox::onValueChangedByStep, [](int value) {
                            nb::Error::ErrorManager::instance().report(nb::Error::Type::INFO, std::to_string(value));

                            if (!activeSceneGraphNode)
                            {
                                return;
                            }

                            activeSceneGraphNode->addTranslate({ 0.0f, (float)value, 0.0f });
                        })
                    )
                )

                // Поле Z
                .child(LayoutBuilder::vBox().relativeWidth(0.33f)
                    .child(LayoutBuilder::widget(new Widgets::SpinBox())
                        .apply<Widgets::SpinBox>([](Widgets::SpinBox* c) {
                            c->setRange(-100, 100);
                        })
                        .relativeWidth(1.0f)
                        .absoluteHeight(30)
                        .background(NbColor{ 25, 25, 25 })
                        .color(NbColor{ 100, 100, 255 }) 
                        .onEvent(&Widgets::SpinBox::onValueChangedByStep, [](int value) {
                            nb::Error::ErrorManager::instance().report(nb::Error::Type::INFO, std::to_string(value));

                            if (!activeSceneGraphNode)
                            {
                                return;
                            }

                            activeSceneGraphNode->addTranslate({ 0.0f, 0.0f, (float)value });
                        })
                    )
                )
            )
        )

        // Можно добавить Rotation аналогично
        //.child(
        //    LayoutBuilder::hBox()
        //    .absoluteHeight(30)
        //    .margin({ 0, 0, 5, 0 })
        //    .child(LayoutBuilder::label(L"Rotation")
        //        .relativeWidth(0.35f)
        //        .color(NbColor{ 180, 180, 180 })
        //    )
        //    .child(LayoutBuilder::hBox()
        //        .relativeWidth(0.65f)
        //        //.child(LayoutBuilder::widget(new Widgets::SpinBox()).relativeWidth(0.33f).background(NbColor{ 25, 25, 25 }))
        //        //.child(LayoutBuilder::widget(new Widgets::SpinBox()).relativeWidth(0.33f).background(NbColor{ 25, 25, 25 }))
        //        //.child(LayoutBuilder::widget(new Widgets::SpinBox()).relativeWidth(0.33f).background(NbColor{ 25, 25, 25 }))
        //    )
        //)
        .build();

        
    child3->getLayoutRoot()->addChild(std::move(inspector));

    g_engine = std::make_shared<nb::Core::Engine>(sceneWindow->getHandle().as<HWND>());
    sceneWindow->show();
    child1->show();
    child2->show();
    child3->show();
    //child4->show();
    //child4->repaint();
    childWnd5->show();
    childWnd6.show();
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

    g_engine->getRenderer()->createSharedContextForWindow(childWnd5->getHandle().as<HWND>());
    childWnd5->setRenderable(false);

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

            sceneWindow->setRenderable(false);

            auto rootChild1 = child1->getLayoutRoot();
            std::shared_ptr<SceneModel> sceneModel = std::make_shared<SceneModel>(g_engine->getRenderer()->getScene());

            auto uichild1 = LayoutBuilder::hBox()
                .style([](NNsLayout::LayoutStyle& s) {
                s.padding = { 10, 10, 10, 10 };
                s.margin = { 50,50,50,50 };
                s.color = NbColor{ 30, 30, 30 };
                s.heightSizeType = NNsLayout::SizeType::RELATIVE;
                s.height = 1.0f;
                    })
                .child(LayoutBuilder::widget(new Widgets::TreeView({}))
                    .setTreeModel(sceneModel)
                    .text(L"NONE")
                    .relativeWidth(1.0f)
                    .relativeHeight(1.0f)
                    .margin({ 5, 5, 5, 5 })
                    .background(NbColor{ 70, 70, 70 })
                    .color(NbColor{ 200, 200, 200 })
                    .onEvent(&Widgets::TreeView::onItemClickSignal, [&sceneModel](const Widgets::ModelIndex& index) {
                        if (!sceneModel)
                        {
                            return;
                        }

                        if(!index.isValid())
                        {   
                            return;
                        }

                        activeSceneGraphNode = reinterpret_cast<nb::Renderer::BaseNode*>(sceneModel->findById(index.getUuid())->getData());
                    })
                )
                .build();

            rootChild1->addChild(std::move(uichild1));
            child1->show();

            notInit = true;
            //childWnd2.repaint(); // этому коду срочно нужен рефакторинг)))))) 

            
           

            
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
            if (msg.message != WM_INPUT)
            {
                //std::string str = MsgToString(msg);
                //Debug::debug(str);

            }
            if (shouldShowDebug)
            {
               

            }
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
    _CrtDumpMemoryLeaks();  // Явный вывод

    return (int)msg.wParam;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////