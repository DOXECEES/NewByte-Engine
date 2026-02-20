#ifndef SDK_APP_HPP
#define SDK_APP_HPP

#include <Windows.h>

// ENGINE HEADERS
#include <Core/Engine.hpp>

// NBUI HEADERS

#include <Win32Window/Win32Window.hpp>
#include <Win32Window/Win32ChildWindow.hpp>
#include <TempDocking.hpp>

#include <memory>
#include <string>
#include <vector>
#include <atomic>

#include "SceneModel.hpp"

namespace nbui
{
    class LayoutBuilder;
};

class EditorApp 
{
public:
    EditorApp() : running(true), activeNode(nullptr) {}

    int run(::HINSTANCE hInstance)
    {
        initSystems();
        createWindows();
        setupDocking();
        initEngine();

        setupHierarchyUI();
        setupInspectorUI();
        setupSettingsUI();

        showAllWindows();

        return mainLoop();
    }

private:

    bool isEngineDependentUiInit = false;

    std::shared_ptr<nb::Core::Engine> engine;
    std::shared_ptr<Win32Window::Window> mainWindow;
    std::unique_ptr<Temp::DockingSystem> dockManager;

    std::shared_ptr<Win32Window::ChildWindow> sceneWindow;
    std::shared_ptr<Win32Window::ChildWindow> hierarchyWindow;
    std::shared_ptr<Win32Window::ChildWindow> inspectorWindow;
    std::shared_ptr<Win32Window::ChildWindow> settingsWindow;
    std::shared_ptr<Win32Window::ChildWindow> debugWindow;
    std::shared_ptr<Win32Window::ChildWindow> textureInspector;

    std::shared_ptr<SceneModel> sceneModel;
    nb::Renderer::BaseNode* activeNode;
    std::atomic<bool> running;

    void initSystems() noexcept;
  
    void setAppLocale() noexcept;
    
    void createWindows() noexcept;
    
    void setupDocking() noexcept;

    void initEngine() noexcept;
    
    void setupHierarchyUI() noexcept;
    
    void setupInspectorUI() noexcept;
    
    nbui::LayoutBuilder createSpinBox(std::function<void(int)> onChange, const NbColor& color);

    void setupSettingsUI() noexcept;
    
    void setupEngineDependentUi() noexcept;

    void setupDebugUI() noexcept;
   

    void showAllWindows()
    {
        sceneWindow->show();
        hierarchyWindow->show();
        inspectorWindow->show();
        settingsWindow->show();
        debugWindow->show();
        textureInspector->show();
        mainWindow->show();
        mainWindow->repaint();
    }

    int mainLoop() {
        MSG msg = { 0 };
        while (running)
        {

            if (engine 
                && engine->getRenderer()->isResourceReady() 
                && !isEngineDependentUiInit)
            {
                setupEngineDependentUi();
                isEngineDependentUiInit = true;
            }

            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT) {
                    running = false;
                    break;
                }

                if (msg.message == WM_INPUT) {
                    engine->bufferizeInput(msg);
                }

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            if (engine)
            {
                
                engine->processInput();
                engine->run(!sceneWindow->getIsRenderable());

                if (engine->getMode() == nb::Core::Engine::Mode::EDITOR)
                    mainWindow->showCursor();
                else
                    mainWindow->hideCursor();
            }

            mainWindow->resetStateDirtyFlags();
            sceneWindow->resetStateDirtyFlags();
        }
        return static_cast<int>(msg.wParam);
    }
};

#endif
