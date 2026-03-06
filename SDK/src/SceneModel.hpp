#ifndef SDK_SCENEMODEL_HPP
#define SDK_SCENEMODEL_HPP

#include <NbCore.hpp>

#include <ECS/ecs.hpp>
#include <Renderer/Scene.hpp>

#include <Widgets/TreeView.hpp>

#include <stack>
#include <memory>


class SceneModelEcs final : public Widgets::ITreeModel
{
public:

    explicit SceneModelEcs(
        nb::Ecs::ECSRegistry& registry,
        nb::Ecs::EntityID rootEntity
    )
        : ecs(registry),
          root(rootEntity)
    {
        buildModel();
    }

    ~SceneModelEcs() override = default;

    // ===== ITreeModel =====

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
        nb::Ecs::EntityID id = getEntity(item);
        nb::Ecs::Entity entity{id};

        if (!ecs.has<NameComponent>(entity))
        {
            return "Entity";
        }

        return ecs.get<NameComponent>(entity).name;
    }

    size_t size() const noexcept override
    {
        return uuidMap.size();
    }

    // ===== API =====

    void rebuildFromScene() noexcept
    {
        rootItems.clear();
        uuidMap.clear();
        buildModel();
    }

    nb::Ecs::EntityID getEntity(const Widgets::ModelIndex& index) const noexcept
    {
        const Widgets::ModelItem* item = uuidMap.at(index.getUuid());
        return getEntity(*item);
    }

    nb::Ecs::EntityID getEntity(const Widgets::ModelItem& item) const noexcept
    {
        return static_cast<nb::Ecs::EntityID>(reinterpret_cast<uintptr_t>(item.getData()));
    }

private:
    nb::Ecs::ECSRegistry& ecs;
    nb::Ecs::EntityID root;

    std::vector<std::unique_ptr<Widgets::ModelItem>> rootItems;
    std::unordered_map<nbstl::Uuid, Widgets::ModelItem*> uuidMap;

private:
    void buildModel() noexcept
    {
        //if (!ecs.iisValid(root))
        //{
        //    return;
        //}

        // создаём корень
        rootItems.push_back(
            std::make_unique<Widgets::ModelItem>(
                reinterpret_cast<void*>(static_cast<uintptr_t>(root)), nullptr, 0
            )
        );

        Widgets::ModelItem* rootItem = rootItems.back().get();
        uuidMap[rootItem->getUuid()] = rootItem;

        nb::Ecs::Entity entity{root};
        if (!ecs.has<HierarchyComponent>(entity))
        {
            return;
        }

        struct StackItem
        {
            nb::Ecs::EntityID entity;
            Widgets::ModelItem* modelItem;
            size_t depth;
        };

        std::stack<StackItem> stk;
        stk.push({root, rootItem, 0});

        while (!stk.empty())
        {
            auto [entityId, modelItem, depth] = stk.top();
            stk.pop();

            nb::Ecs::Entity entity{entityId};
            if (!ecs.has<HierarchyComponent>(entity))
            {
                continue;
            }

            auto& hierarchy = ecs.get<HierarchyComponent>(entity);

            for (nb::Ecs::EntityID child : hierarchy.children)
            {
                auto childItem = std::make_unique<Widgets::ModelItem>(
                    reinterpret_cast<void*>(static_cast<uintptr_t>(child)), modelItem, depth + 1
                );

                Widgets::ModelItem* rawPtr = childItem.get();

                modelItem->children.push_back(std::move(childItem));
                uuidMap[rawPtr->getUuid()] = rawPtr;

                stk.push({child, rawPtr, depth + 1});
            }
        }
    }
};



#endif