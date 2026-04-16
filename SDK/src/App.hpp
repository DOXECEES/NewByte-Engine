#ifndef SDK_APP_HPP
#define SDK_APP_HPP
#define TINYGIZMO_IMPLEMENTATION
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

#include "Renderer/IRenderAPI.hpp"
#include "Renderer/Renderer.hpp"
#include "SceneModel.hpp"
#include "TextureEditor.hpp"
#include "AssetManger.hpp"
#include "MaterialEditor.hpp"
//
#include <Win32Window/Win32ModalWindow.hpp>
#include <tiny-gizmo.hpp>
//


namespace nbui
{
    class LayoutBuilder;
};

class EditorApp 
{
public:
    EditorApp() : running(true), activeNode() {}

    int run(::HINSTANCE hInstance)
    {
        initSystems();
        createWindows();
        setupDocking();
        initEngine();
        setupMainWindow();
        setupHierarchyUI();
        //setupSettingsUI();

        showAllWindows();

        subscribeAll();
        //openFilePickerWindow();
        //openColorPickerWindow();

      


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
    std::shared_ptr<Win32Window::ChildWindow> debugWindow;
    std::shared_ptr<Win32Window::ChildWindow> assetManager;
    std::shared_ptr<Win32Window::ChildWindow> toolbarWindow;
    //std::shared_ptr<Win32Window::ChildWindow> tempWindow;


    //
    std::shared_ptr<Win32Window::ModalWindow> colorPickerWindow;
    std::shared_ptr<Win32Window::ModalWindow> filePickerWindow;

    std::shared_ptr<AssetManager> assetManagerWindow;
    std::shared_ptr<MaterialEditor> materialEditor;

    Widgets::TreeView* savedTreeView = nullptr; 


    void openColorPickerWindow();
    void openFilePickerWindow();

    //
    std::shared_ptr<SceneModelEcs> sceneModel;
    //nb::Renderer::BaseNode* activeNode = nullptr;
    nb::Node activeNode;
    std::atomic<bool> running;

    bool shouldRebuildInspector = false; 

    Signal<void()> onActiveNodeChanged;

    void initSystems() noexcept;
  
    void setAppLocale() noexcept;
    
    void createWindows() noexcept;
    
    void setupDocking() noexcept;

    void initEngine() noexcept;
    
    void setupMainWindow() noexcept;

    void setupHierarchyUI() noexcept;
    
    void setupInspectorUI() noexcept;
    
    nbui::LayoutBuilder createSpinBox(std::function<void(int)> onChange, const NbColor& color);

    void setupSettingsUI() noexcept;
    
    void setupEngineDependentUi() noexcept;

    void setupDebugUI() noexcept;
    void setupAssetManager() noexcept;
   
    void rebuildInspector() noexcept;

    void subscribeAll() noexcept;

    void markComponentDirty(
        void* componentPtr,
        const nb::Reflect::TypeInfo* typeInfo
    ) noexcept;

    nbui::LayoutBuilder buildFieldUI(
        nbui::LayoutBuilder parent,
        void* componentPtr,
        const nb::Reflect::TypeInfo* info,
        const nb::Reflect::FieldInfo& field
    ) noexcept;

    void showAllWindows()
    {
        sceneWindow->show();
        hierarchyWindow->show();
        inspectorWindow->show();
        debugWindow->show();
        //textureInspector->show();
        mainWindow->show();
        mainWindow->repaint();
    }


    void addCubeToEntity(
        const Widgets::ModelIndex& index,
        Widgets::TreeView*         tv
    ) noexcept; 

    void addSphereToEntity(
        const Widgets::ModelIndex& index,
        Widgets::TreeView*         tv
    ) noexcept;

   void setupHierarchyEvents(Widgets::TreeView* tv) noexcept;


    int mainLoop() {
        MSG msg = { 0 };
        while (running)
        {

            if (engine 
                && engine->getRenderer()->isResourceReady() 
                && !isEngineDependentUiInit)
            {
                setupEngineDependentUi();
                assetManagerWindow = std::make_shared<AssetManager>(assetManager, engine.get());
                isEngineDependentUiInit = true;
            }

            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                if (shouldRebuildInspector)
                {
                    rebuildInspector();
                    shouldRebuildInspector = false;
                }

                if (msg.message == WM_QUIT) {
                    running = false;
                    break;
                }
                bool isGizmoHit = false;

                if (msg.hwnd == sceneWindow->getHandle().as<HWND>() && engine)
                {
                    //
                    NbPoint<int> mousePos = sceneWindow->mousePosition;

                    nb::Renderer::Camera* camera = engine->getRenderer()->getCamera();
                    nb::Math::Ray ray = camera->getRayFromMousePoint(mousePos.x, mousePos.y);

                    auto& gizmo_ctx = engine->getRenderer()->getGizmoContext();
                    tinygizmo::gizmo_application_state state;
                    state.ray_origin = {ray.origin.x, ray.origin.y, ray.origin.z};
                    state.ray_direction = {ray.direction.x, ray.direction.y, ray.direction.z};

                    state.hotkey_ctrl = GetAsyncKeyState(VK_CONTROL) & 0x8000;
                    state.mouse_left = sceneWindow->leftMouseClicked;
                    state.hotkey_translate = GetAsyncKeyState(VK_F1) & 0x8000;
                    state.hotkey_rotate = GetAsyncKeyState(VK_F2) & 0x8000;
                    state.hotkey_scale = GetAsyncKeyState(VK_F3) & 0x8000;

                    gizmo_ctx.update(state);

                    if (activeNode.isValid())
                    {
                        auto& tc = activeNode.getComponent<TransformComponent>();

                        tinygizmo::rigid_transform t;
                        t.position = {tc.position.x, tc.position.y, tc.position.z};
                        t.scale = {tc.scale.x, tc.scale.y, tc.scale.z};

                        auto quat = tc.rotation;
                        quat.normalize();

                        t.orientation = {-quat.x, -quat.y, -quat.z, quat.w};

                        if (tinygizmo::transform_gizmo("object_gizmo", gizmo_ctx, t))
                        {
                            tc.position = {t.position.x, t.position.y, t.position.z};
                            tc.scale = {t.scale.x, t.scale.y, t.scale.z};

                            nb::Math::Quaternion resultQuat(
                                -t.orientation.x, -t.orientation.y, -t.orientation.z, t.orientation.w
                            );

                            if (resultQuat.w < 0.0f)
                            {
                                resultQuat.x = -resultQuat.x;
                                resultQuat.y = -resultQuat.y;
                                resultQuat.z = -resultQuat.z;
                                resultQuat.w = -resultQuat.w;
                            }
                            resultQuat.normalize();

                            tc.rotation = resultQuat;

                            tc.eulerAngle = tc.rotation.toEulerXYZ();
                            tc.lastEuler  = tc.eulerAngle; 


                            tc.dirty = true;
                            tc.physicsDirty = true;
                            isGizmoHit = true;
                        }
                    }


                    //
                    if (msg.message == WM_LBUTTONDOWN && !isGizmoHit)
                    {
                        NbPoint<int> point = {GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam)};

                        nb::Math::Ray ray;
                        nb::Node node = engine->rayPick(point.x, point.y, ray);

                        if (node.isValid())
                        {
                            activeNode = node;
                        }
                        else
                        {
                            activeNode = nb::Node::createInvalid(); 
                        }

                        onActiveNodeChanged.emit();
                    }

                    isGizmoHit = false;
                }

                if (msg.message == WM_INPUT) 
                {
                    engine->bufferizeInput(msg);
                }

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            if (engine)
            {               
                engine->processInput();
                engine->run(!sceneWindow->getIsRenderable());
 
                if (engine->shouldHideCursor())
                    mainWindow->hideCursor();
                else
                    mainWindow->showCursor();
            }

            mainWindow->resetStateDirtyFlags();
            sceneWindow->resetStateDirtyFlags();
        }
        return static_cast<int>(msg.wParam);
    }
};

#endif
