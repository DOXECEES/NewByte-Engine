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
#include <unordered_set>

#include "Renderer/IRenderAPI.hpp"
#include "Renderer/Renderer.hpp"
#include "SceneModel.hpp"
#include "TextureEditor.hpp"
#include "AssetManger.hpp"
#include "MaterialEditor.hpp"
#include "ImportWindow.hpp"
//
#include <Win32Window/Win32ModalWindow.hpp>
#include "FramebufferVisualization.hpp"
#include <tiny-gizmo.hpp>
//
#include <Utils/PrimitiveNameManager.hpp>

#include "CameraBookmark.hpp"
#include "CameraBookmarkWindow.hpp"

#include "Camera/CameraSettingsController.hpp"
#include "EngineSettingsController.hpp"

#include "Scene/SceneWindow.hpp"
#include "Scene/SceneController.hpp"

namespace nbui
{
    class LayoutBuilder;
};

class EditorApp 
{
public:
    EditorApp() : running(true) {}

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

    struct SpawnModelParams
    {
        nb::Math::Vector3<float> position;
        std::filesystem::path    pathToModel;
    };

    static void requestModelSpawn(
        const SpawnModelParams&      params
    ) noexcept;

private:

    inline static nbstl::Vector<SpawnModelParams> spawnQueue;

    bool isEngineDependentUiInit = false;

    std::shared_ptr<nb::Core::Engine> engine;
    std::shared_ptr<Win32Window::Window> mainWindow;
    std::unique_ptr<Temp::DockingSystem> dockManager;

    std::shared_ptr<sdk::SceneWindow> sceneWindow = nullptr;
    std::shared_ptr<sdk::SceneController> sceneController = nullptr;

    // std::shared_ptr<Win32Window::ChildWindow> sceneTabWindow;
    // std::shared_ptr<Win32Window::ChildWindow> sceneToolbar;
    // std::shared_ptr<Win32Window::ChildWindow> sceneWindow;

    std::shared_ptr<Win32Window::ChildWindow> hierarchyWindow;
    std::shared_ptr<Win32Window::ChildWindow> inspectorWindow;
    std::shared_ptr<Win32Window::ChildWindow> debugWindow;
    std::shared_ptr<Win32Window::ChildWindow> assetManager;
    std::shared_ptr<Win32Window::ChildWindow> toolbarWindow;
    std::unique_ptr<sdk::CameraBookmarkWindow> cameraBookmarkWindow;

    //std::shared_ptr<Win32Window::ChildWindow> tempWindow;
    std::shared_ptr<Win32Window::ChildWindow> previewWindow;
    nb::Renderer::SharedWindowContext         sharedContext;
    //
    std::shared_ptr<Win32Window::ModalWindow> colorPickerWindow;
    std::shared_ptr<Win32Window::ModalWindow> filePickerWindow;
    std::shared_ptr<Win32Window::ModalWindow> languagePicker;
    std::shared_ptr<Win32Window::ModalWindow> gizmoToggleWindow = nullptr;


    std::shared_ptr<Win32Window::ChildWindow> shaderNodes;
    std::shared_ptr<Sdk::FramebufferVisualization> framebufferVisualization;



    std::shared_ptr<AssetManager> assetManagerWindow;
    std::shared_ptr<MaterialEditor> materialEditor;
    std::shared_ptr<ImportWindow> importWindow = nullptr;


    Widgets::TreeView* savedTreeView = nullptr; 
    nb::Ecs::EntityID  copiedEntityId;
    sdk::CameraBookmarkManager cameraBookmarkManager;
    sdk::CameraSettingsController cameraSettingsController;

    std::unique_ptr<sdk::EngineSettingsController> engineSettingsController;
    nb::Renderer::DebugRendererSettings debugRendererSettings = {};

    void openColorPickerWindow();
    void openFilePickerWindow();
    void openFilePicker(
        const std::wstring&                     title,
        std::function<void(const std::string&)> onSelected,
        Win32Window::IWindow*                   parent,
        const std::vector<std::string>&         extentions = {}
    );

    nbui::LayoutBuilder createMenuButton(
        const std::string& labelKey, 
        std::function<void(nbui::PopupMenu*)> populateMenuFunc
    ) noexcept;
   


    void refreshInterfaceText() noexcept;

    //
    std::shared_ptr<SceneModelEcs> sceneModel;
    //nb::Renderer::BaseNode* activeNode = nullptr;
    ///nb::Node activeNode;
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

    void spawnEmpty(const Widgets::ModelIndex& index) noexcept;
    void spawnModel(
        const Widgets::ModelIndex&      index,
        const std::filesystem::path&    pathToModel,
        const nb::Math::Vector3<float>& position
    ) noexcept;

    void showAllWindows()
    {
        sceneWindow->getViewportWindow()->show();
        hierarchyWindow->show();
        inspectorWindow->show();
        debugWindow->show();
        //textureInspector->show();

        mainWindow->show();
        mainWindow->repaint();
    }

    void setupHierarchyEvents(Widgets::TreeView* tv) noexcept;

    void deleteEntity(const Widgets::ModelIndex& index) noexcept;
    void copyEntity(const Widgets::ModelIndex& index) noexcept;
    void pasteEntity(const Widgets::ModelIndex& index) noexcept;

    void releaseNamesRecursive(nb::Ecs::EntityID id) noexcept;
    std::unordered_set<std::string> m_existingPreviews;
    
    // Пути к материалам, превью для которых прямо сейчас генерируется
    std::unordered_set<std::string> m_pendingPreviews;


    int mainLoop()
    {
        MSG  msg                    = {0};
        bool leftMouseDownThisFrame = false;

        auto sceneWindowViewport = sceneWindow->getViewportWindow();

        while (running)
        {
            if (engine && engine->getRenderer()->isResourceReady() && !isEngineDependentUiInit)
            {
                setupEngineDependentUi();
                assetManagerWindow = std::make_shared<AssetManager>(assetManager, engine.get());
                isEngineDependentUiInit = true;
            }

            // Очищаем флаг клика перед обработкой сообщений
            leftMouseDownThisFrame = false;

            // 1. ОЧЕНЬ БЫСТРЫЙ СБОР СООБЩЕНИЙ (БЕЗ МАТЕМАТИКИ)
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                if (shouldRebuildInspector)
                {
                    rebuildInspector();
                    shouldRebuildInspector = false;
                }

                if (msg.message == WM_QUIT)
                {
                    running = false;
                    break;
                }

                if (msg.hwnd == sceneWindowViewport->getHandle().as<HWND>())
                {
                    if (msg.message == WM_LBUTTONDOWN)
                    {
                        leftMouseDownThisFrame = true;
                    }
                }

                if (msg.message == WM_INPUT)
                {
                    engine->bufferizeInput(msg);
                }

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            // Если очередь пуста, обрабатываем физику, логику и ГИЗМО строго 1 раз за кадр
            if (engine && running)
            {
                bool isGizmoHit = false;

                // Выполняем спавн моделей
                if (!spawnQueue.isEmpty())
                {
                    for (auto& i : spawnQueue)
                    {
                        sceneController->spawnModel(Widgets::ModelIndex(), i.pathToModel, i.position);
                    }
                    spawnQueue.clear();
                }

                // 2. ОБНОВЛЕНИЕ ГИЗМО (ВЫПОЛНЯЕТСЯ СТРОГО 1 РАЗ ЗА КАДР)
                if (!sceneWindowViewport->getIsRenderable() && sceneController) // обновляем только если окно активно
                {
                    NbPoint<int>          mousePos = sceneWindowViewport->mousePosition;
                    nb::Renderer::Camera* camera   = engine->getRenderer()->getCamera();
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

                    if (auto activeNode = sceneController->getActiveNode(); activeNode.isValid())
                    {
                        auto& tc = activeNode.getComponent<TransformComponent>();

                        nb::Math::Mat4<float> pWorldMatrix      = nb::Math::Mat4<float>::identity();
                        nb::Math::Quaternion<float> pWorldRot   = {0, 0, 0, 1};
                        nb::Math::Vector3<float>    pWorldScale = {1, 1, 1};
                        bool                        hasParent   = false;

                        if (auto parent = activeNode.getParent();
                            parent.has_value() && parent->hasComponent<TransformComponent>())
                        {
                            auto& ptc    = parent->getComponent<TransformComponent>();
                            pWorldMatrix = ptc.worldMatrix;
                            pWorldRot    = nb::Math::getRotationFromModelMatrix(pWorldMatrix);
                            pWorldScale  = nb::Math::getScaleFromModelMatrix(pWorldMatrix);
                            hasParent    = true;
                        }

                        nb::Math::Vector3<float> currentWorldPos;
                        if (hasParent)
                        {
                            currentWorldPos.x = tc.position.x * pWorldMatrix[0][0] +
                                                tc.position.y * pWorldMatrix[1][0] +
                                                tc.position.z * pWorldMatrix[2][0] +
                                                pWorldMatrix[3][0];
                            currentWorldPos.y = tc.position.x * pWorldMatrix[0][1] +
                                                tc.position.y * pWorldMatrix[1][1] +
                                                tc.position.z * pWorldMatrix[2][1] +
                                                pWorldMatrix[3][1];
                            currentWorldPos.z = tc.position.x * pWorldMatrix[0][2] +
                                                tc.position.y * pWorldMatrix[1][2] +
                                                tc.position.z * pWorldMatrix[2][2] +
                                                pWorldMatrix[3][2];
                        }
                        else
                        {
                            currentWorldPos = tc.position;
                        }

                        nb::Math::Quaternion<float> worldQuat =
                            hasParent ? (tc.rotation * pWorldRot) : tc.rotation;

                        nb::Math::Vector3<float> worldScale = {
                            tc.scale.x * pWorldScale.x, tc.scale.y * pWorldScale.y,
                            tc.scale.z * pWorldScale.z
                        };

                        tinygizmo::rigid_transform t;
                        t.position = {currentWorldPos.x, currentWorldPos.y, currentWorldPos.z};
                        t.scale    = {worldScale.x, worldScale.y, worldScale.z};

                        nb::Math::Quaternion<float> inputQuat = worldQuat.conjugate();
                        t.orientation = {inputQuat.x, inputQuat.y, inputQuat.z, inputQuat.w};

                        bool gizmoInteracted =
                            tinygizmo::transform_gizmo("object_gizmo", gizmo_ctx, t);

                        if (gizmoInteracted)
                        {
                            isGizmoHit = true;
                            if (state.mouse_left)
                            {
                                nb::Math::Vector3<float> G_worldPos(
                                    t.position.x, t.position.y, t.position.z
                                );
                                nb::Math::Vector3<float> G_worldScale(
                                    t.scale.x, t.scale.y, t.scale.z
                                );
                                nb::Math::Quaternion<float> outputQuat(
                                    t.orientation.x, t.orientation.y, t.orientation.z,
                                    t.orientation.w
                                );

                                nb::Math::Quaternion<float> newWorldQuat = outputQuat.conjugate();

                                float dot =
                                    newWorldQuat.x * worldQuat.x + newWorldQuat.y * worldQuat.y +
                                    newWorldQuat.z * worldQuat.z + newWorldQuat.w * worldQuat.w;
                                if (dot < 0.0f)
                                {
                                    newWorldQuat.x = -newWorldQuat.x;
                                    newWorldQuat.y = -newWorldQuat.y;
                                    newWorldQuat.z = -newWorldQuat.z;
                                    newWorldQuat.w = -newWorldQuat.w;
                                }

                                sdk::Snapper& snapper = sceneController->getSnapper();

                                G_worldPos = snapper.calculateTranslation(G_worldPos);
                                newWorldQuat = snapper.calculateRotation(newWorldQuat);
                                G_worldScale = snapper.calculateScale(G_worldScale);
                                
                                if (hasParent)
                                {
                                    nb::Math::Mat4<float> invParent =
                                        nb::Math::inverseWithoutTranspose(pWorldMatrix);
                                    tc.position.x = G_worldPos.x * invParent[0][0] +
                                                    G_worldPos.y * invParent[1][0] +
                                                    G_worldPos.z * invParent[2][0] +
                                                    invParent[3][0];
                                    tc.position.y = G_worldPos.x * invParent[0][1] +
                                                    G_worldPos.y * invParent[1][1] +
                                                    G_worldPos.z * invParent[2][1] +
                                                    invParent[3][1];
                                    tc.position.z = G_worldPos.x * invParent[0][2] +
                                                    G_worldPos.y * invParent[1][2] +
                                                    G_worldPos.z * invParent[2][2] +
                                                    invParent[3][2];

                                    tc.rotation = newWorldQuat * pWorldRot.conjugate();

                                    tc.scale.x = (std::abs(pWorldScale.x) > 0.0001f)
                                                     ? (G_worldScale.x / pWorldScale.x)
                                                     : G_worldScale.x;
                                    tc.scale.y = (std::abs(pWorldScale.y) > 0.0001f)
                                                     ? (G_worldScale.y / pWorldScale.y)
                                                     : G_worldScale.y;
                                    tc.scale.z = (std::abs(pWorldScale.z) > 0.0001f)
                                                     ? (G_worldScale.z / pWorldScale.z)
                                                     : G_worldScale.z;
                                }
                                else
                                {
                                    tc.position = G_worldPos;
                                    tc.rotation = newWorldQuat;
                                    tc.scale    = G_worldScale;
                                }

                                tc.rotation.normalize();
                                tc.eulerAngle =
                                    tc.rotation.toEulerXYZ(); 
                                                              
                                tc.dirty        = true;
                                tc.physicsDirty = true;

                                if (state.hotkey_ctrl)
                                {
                                    nb::Scene::getInstance().snapToSurface(activeNode.getId(), 1000.0f, true);
                                }
                            }
                        }
                    }

                    if (leftMouseDownThisFrame && !isGizmoHit)
                    {
                        nb::Math::Ray pickRay;
                        nb::Node      node = engine->rayPick(mousePos.x, mousePos.y, pickRay);
                        sceneController->setActiveNode(node.isValid() ? node : nb::Node::createInvalid());
                        onActiveNodeChanged.emit();
                    }
                }

                
                engine->processInput();
                {

                    using KeyCode = nb::Input::Keyboard::KeyCode;
                    auto keyboard = engine->keyboard;

                    for(size_t i = 0; i < sdk::CameraBookmarkManager::MAX_BOOKMARKS; i++)
                    {
                        KeyCode targetKey = static_cast<KeyCode>(static_cast<uint8_t>(KeyCode::NB_0) + i);

                        if(keyboard->isKeyHeld(KeyCode::NB_CONTROL) && keyboard->isKeyPressed(targetKey))
                        {
                            if(cameraBookmarkManager.hasBookmark(i))
                            {
                                cameraBookmarkManager.apply(i, engine->getRenderer()->getCamera());
                            }
                            else
                            {
                                cameraBookmarkManager.record(i, engine->getRenderer()->getCamera());
                                cameraBookmarkWindow->refreshUi();
                            }
                        }
                    }

                    
                }
                engine->run(!sceneWindowViewport->getIsRenderable());

                //engine->getRenderer()->renderShadowPreview(
                //    sharedContext, engine->getRenderer()->ssaoResult, 0.1f, 100.0f
                //);

                if (engine->shouldHideCursor())
                {
                    mainWindow->hideCursor();
                }
                else
                {
                    mainWindow->showCursor();
                }
            }

            mainWindow->resetStateDirtyFlags();
            sceneWindowViewport->resetStateDirtyFlags();
        }
        return static_cast<int>(msg.wParam);
    }
};

#endif
