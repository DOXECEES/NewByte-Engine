#ifndef SDK_SCENEMODEL_HPP
#define SDK_SCENEMODEL_HPP

#include <NbCore.hpp>

#include <Renderer/SceneGraph.hpp>

#include <Widgets/TreeView.hpp>

#include <stack>
#include <memory>

class SceneModel final : public Widgets::ITreeModel
{
public:
    explicit SceneModel(std::shared_ptr<nb::Renderer::SceneGraph> sceneGraph)
        : sceneGraph(std::move(sceneGraph))
    {
        buildModel();
    }

    ~SceneModel() override = default;


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

#endif