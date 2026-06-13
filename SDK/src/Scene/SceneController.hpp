#ifndef SDK_SCENE_SCENECONTROLLER_HPP
#define SDK_SCENE_SCENECONTROLLER_HPP

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <Renderer/Material.hpp>
#include <Renderer/Mesh.hpp>
#include <Renderer/Scene.hpp>
#include <Signal.hpp>
#include <Widgets/TreeView.hpp>

class SceneModelEcs;

namespace nb::Utils
{
    class PrimitiveNameManager;
};

namespace nb::Core
{
    class Engine;
}

namespace nb::Reflect
{
    struct TypeInfo;
}

namespace sdk
{

    class SceneController
    {
    public:
        explicit SceneController(
            const std::shared_ptr<nb::Core::Engine>& engine,
            const std::shared_ptr<SceneModelEcs>&    sceneModel,
            nb::Utils::PrimitiveNameManager&                    nameManager
        ) noexcept;

        void spawnPrimitive(
            const Widgets::ModelIndex&   index,
            const void*                  data,
            const nb::Reflect::TypeInfo* typeInfo
        ) noexcept;

        void spawnEmpty(const Widgets::ModelIndex& index) noexcept;

        void spawnModel(
            const Widgets::ModelIndex&      index,
            const std::filesystem::path&    pathToModel,
            const nb::Math::Vector3<float>& position
        ) noexcept;

        void deleteEntity(const Widgets::ModelIndex& index) noexcept;
        void copyEntity(const Widgets::ModelIndex& index) noexcept;
        void pasteEntity(const Widgets::ModelIndex& index) noexcept;

        void markComponentDirty(
            void*                        componentPtr,
            const nb::Reflect::TypeInfo* typeInfo
        ) noexcept;

        [[nodiscard]] nb::Node getActiveNode() const noexcept
        {
            return activeNode;
        }
        void setActiveNode(nb::Node node) noexcept
        {
            activeNode = node;
        }

        [[nodiscard]] Signal<void()>& getActiveNodeChangedSignal() noexcept
        {
            return activeNodeChanged;
        }
        [[nodiscard]] Signal<void()>& getRefreshHierarchySignal() noexcept
        {
            return refreshHierarchySignal;
        }

    private:
        void releaseNamesRecursive(nb::Ecs::EntityID id) noexcept;

        std::shared_ptr<nb::Core::Engine> engine;
        std::shared_ptr<SceneModelEcs>    sceneModel;
        nb::Utils::PrimitiveNameManager&             nameManager;

        nb::Node          activeNode     = nb::Node::createInvalid();
        nb::Ecs::EntityID copiedEntityId = 0;

        Signal<void()> activeNodeChanged;
        Signal<void()> refreshHierarchySignal;
    };
}; // namespace sdk

#endif // SCENE_CONTROLLER_HPP