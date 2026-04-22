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
#include <Utils/PrimitiveNameManager.hpp>

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
    nb::Utils::PrimitiveNameManager primitiveNameManager;

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

    void deleteEntity(const Widgets::ModelIndex& index) noexcept;
    void releaseNamesRecursive(nb::Ecs::EntityID id) noexcept;

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
                    state.ray_origin = {
                        (float)ray.origin.x, (float)ray.origin.y, (float)ray.origin.z
                    };
                    state.ray_direction = {
                        (float)ray.direction.x, (float)ray.direction.y, (float)ray.direction.z
                    };
                    state.mouse_left = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

                    state.hotkey_ctrl      = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
                    state.hotkey_translate = (GetAsyncKeyState(VK_F1) & 0x8000) != 0;
                    state.hotkey_rotate    = (GetAsyncKeyState(VK_F2) & 0x8000) != 0;
                    state.hotkey_scale     = (GetAsyncKeyState(VK_F3) & 0x8000) != 0;

                    gizmo_ctx.update(state);

                    if (activeNode.isValid())
                    {
                        auto& tc = activeNode.getComponent<TransformComponent>();

                        nb::Math::Quaternion<float> pWorldRot;
                        pWorldRot.x = 0.0f;
                        pWorldRot.y = 0.0f;
                        pWorldRot.z = 0.0f;
                        pWorldRot.w = 1.0f;

                        nb::Math::Vector3<float> pWorldScale(1.0f, 1.0f, 1.0f);
                        nb::Math::Vector3<float> pWorldPos(0.0f, 0.0f, 0.0f);
                        bool                     hasParent = false;

                        if (auto parent = activeNode.getParent(); parent.has_value())
                        {
                            auto& ptc   = parent->getComponent<TransformComponent>();
                            pWorldPos   = nb::Math::getPositionFromModelMatrix(ptc.worldMatrix);
                            pWorldRot   = nb::Math::getRotationFromModelMatrix(ptc.worldMatrix);
                            pWorldScale = nb::Math::getScaleFromModelMatrix(ptc.worldMatrix);
                            hasParent   = true;
                        }

                        nb::Math::Vector3<float> worldPos;
                        if (hasParent)
                        {
                            nb::Math::Vector3<float> scaledLocalPos(
                                tc.position.x * pWorldScale.x, tc.position.y * pWorldScale.y,
                                tc.position.z * pWorldScale.z
                            );
                            worldPos = (pWorldRot * scaledLocalPos) + pWorldPos;
                        }
                        else
                        {
                            worldPos = tc.position;
                        }

                        nb::Math::Quaternion<float> worldQuat =
                            hasParent ? (tc.rotation * pWorldRot) : tc.rotation;
                        nb::Math::Vector3<float> worldScale = tc.scale;
                        if (hasParent)
                        {
                            worldScale.x *= pWorldScale.x;
                            worldScale.y *= pWorldScale.y;
                            worldScale.z *= pWorldScale.z;
                        }

                        tinygizmo::rigid_transform t;
                        t.position    = {worldPos.x, worldPos.y, worldPos.z};
                        t.scale       = {worldScale.x, worldScale.y, worldScale.z};
                        t.orientation = {0.0f, 0.0f, 0.0f, 1.0f}; 

                        bool gizmoInteracted =
                            tinygizmo::transform_gizmo("object_gizmo", gizmo_ctx, t);
                        if (gizmoInteracted)
                        {
                            isGizmoHit = true;
                        }

                        if (gizmoInteracted && state.mouse_left)
                        {
                            nb::Math::Vector3<float> G_worldPos(
                                t.position.x, t.position.y, t.position.z
                            );
                            nb::Math::Vector3<float> G_worldScale(t.scale.x, t.scale.y, t.scale.z);

                            nb::Math::Quaternion<float> G_deltaQuat;
                            G_deltaQuat.x = -t.orientation.x;
                            G_deltaQuat.y = -t.orientation.y;
                            G_deltaQuat.z = -t.orientation.z;
                            G_deltaQuat.w = t.orientation.w;

                            if (hasParent)
                            {
                                nb::Math::Vector3<float> deltaPos = G_worldPos - pWorldPos;
                                nb::Math::Vector3<float> unrotatedDelta =
                                    pWorldRot.conjugate() * deltaPos;

                                tc.position.x = (pWorldScale.x != 0)
                                                    ? (unrotatedDelta.x / pWorldScale.x)
                                                    : 0.0f;
                                tc.position.y = (pWorldScale.y != 0)
                                                    ? (unrotatedDelta.y / pWorldScale.y)
                                                    : 0.0f;
                                tc.position.z = (pWorldScale.z != 0)
                                                    ? (unrotatedDelta.z / pWorldScale.z)
                                                    : 0.0f;

                                nb::Math::Quaternion<float> newWorldQuat = G_deltaQuat * worldQuat;
                                tc.rotation = newWorldQuat * pWorldRot.conjugate();

                                tc.scale.x =
                                    (pWorldScale.x != 0) ? (G_worldScale.x / pWorldScale.x) : 1.0f;
                                tc.scale.y =
                                    (pWorldScale.y != 0) ? (G_worldScale.y / pWorldScale.y) : 1.0f;
                                tc.scale.z =
                                    (pWorldScale.z != 0) ? (G_worldScale.z / pWorldScale.z) : 1.0f;
                            }
                            else
                            {
                                tc.position = G_worldPos;
                                tc.rotation = G_deltaQuat * worldQuat;
                                tc.scale    = G_worldScale;
                            }

                            tc.rotation.normalize();
                            tc.eulerAngle   = tc.rotation.toEulerXYZ();
                            tc.lastEuler    = tc.eulerAngle;
                            tc.dirty        = true;
                            tc.physicsDirty = true;
                        }
                    }

                    if (msg.message == WM_LBUTTONDOWN && !isGizmoHit)
                    {
                        NbPoint<int>  point = {GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam)};
                        nb::Math::Ray pickRay;
                        nb::Node      node = engine->rayPick(point.x, point.y, pickRay);
                        activeNode         = node.isValid() ? node : nb::Node::createInvalid();
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
