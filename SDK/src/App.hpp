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
    std::shared_ptr<Win32Window::ChildWindow> previewWindow;
    nb::Renderer::SharedWindowContext         sharedContext;
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

    Signal<void()> refreshHierarchyTreeViewSignal;
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

    void spawnPrimitive(
        const Widgets::ModelIndex& index,
        void*                      data,
        nb::Reflect::TypeInfo*     typeInfo
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
                    NbPoint<int> mousePos = sceneWindow->mousePosition;

                    nb::Renderer::Camera* camera = engine->getRenderer()->getCamera();
                    nb::Math::Ray ray = camera->getRayFromMousePoint(mousePos.x, mousePos.y);

                    auto& gizmo_ctx = engine->getRenderer()->getGizmoContext();

                    tinygizmo::gizmo_application_state state;
                    state.ray_origin    = {ray.origin.x, ray.origin.y, ray.origin.z};
                    state.ray_direction = {ray.direction.x, ray.direction.y, ray.direction.z};

                    state.mouse_left = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

                    state.hotkey_ctrl      = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
                    state.hotkey_translate = (GetAsyncKeyState(VK_F1) & 0x8000) != 0;
                    state.hotkey_rotate    = (GetAsyncKeyState(VK_F2) & 0x8000) != 0;
                    state.hotkey_scale     = (GetAsyncKeyState(VK_F3) & 0x8000) != 0;

                    gizmo_ctx.update(state);

                    static bool dragging = false;

                    if (activeNode.isValid())
                    {
                        auto& tc = activeNode.getComponent<TransformComponent>();

                        nb::Math::Vector3 worldPos =
                            nb::Math::getPositionFromModelMatrix(tc.worldMatrix);

                        nb::Math::Vector3 worldScale =
                            nb::Math::getScaleFromModelMatrix(tc.worldMatrix);

                        nb::Math::Quaternion worldQuat =
                            nb::Math::getRotationFromModelMatrix(tc.worldMatrix);

                        tinygizmo::rigid_transform t;
                        t.position    = {worldPos.x, worldPos.y, worldPos.z};
                        t.scale       = {worldScale.x, worldScale.y, worldScale.z};
                        t.orientation = {-worldQuat.x, -worldQuat.y, -worldQuat.z, worldQuat.w};

                        bool gizmoInteracted =
                            tinygizmo::transform_gizmo("object_gizmo", gizmo_ctx, t);

                        if (gizmoInteracted)
                        {
                            isGizmoHit = true;
                        }

                        if (gizmoInteracted && state.mouse_left)
                        {
                            nb::Math::Vector3<float> G_worldPos{
                                t.position.x, t.position.y, t.position.z
                            };
                            nb::Math::Vector3<float> G_worldScale{t.scale.x, t.scale.y, t.scale.z};
                            nb::Math::Quaternion<float> G_worldQuat(
                                -t.orientation.x, -t.orientation.y, -t.orientation.z,
                                t.orientation.w
                            );

                            if (auto parent = activeNode.getParent(); parent.has_value())
                            {
                                auto& ptc = parent->getComponent<TransformComponent>();

                                nb::Math::Mat4<float> invParent =
                                    nb::Math::inverseWithoutTranspose(ptc.worldMatrix);

                                nb::Math::Mat4<float> targetWorldMat =
                                    nb::Math::Mat4<float>::identity();
                                targetWorldMat = nb::Math::scale(targetWorldMat, G_worldScale);
                                targetWorldMat = targetWorldMat * G_worldQuat.toMatrix4();
                                targetWorldMat = nb::Math::translate(targetWorldMat, G_worldPos);

                                nb::Math::Mat4<float> localMatrix = invParent * targetWorldMat;

                                tc.position = nb::Math::getPositionFromModelMatrix(localMatrix);
                                tc.rotation = nb::Math::getRotationFromModelMatrix(localMatrix);
                                tc.scale    = nb::Math::getScaleFromModelMatrix(localMatrix);
                            }
                            else
                            {
                                tc.position = G_worldPos;
                                tc.rotation = G_worldQuat;
                                tc.scale    = G_worldScale;
                            }

                            tc.rotation.normalize();
                            tc.eulerAngle = tc.rotation.toEulerXYZ();
                            tc.lastEuler  = tc.eulerAngle;

                            tc.dirty        = true;
                            tc.physicsDirty = true;
                        }
                    }

                    if (msg.message == WM_LBUTTONDOWN && !isGizmoHit)
                    {
                        NbPoint<int> point = {GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam)};

                        nb::Math::Ray pickRay;
                        nb::Node      node = engine->rayPick(point.x, point.y, pickRay);

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

                
               
                engine->getRenderer()->renderShadowPreview(
                    sharedContext, engine->getRenderer()->getShadowTextureId(), 0.1f, 100.0f
                );
                

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
