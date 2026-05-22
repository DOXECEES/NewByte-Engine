#ifndef SDK_FILESYSTEMMODEL_HPP
#define SDK_FILESYSTEMMODEL_HPP

#include <Widgets/TreeView.hpp>
#include <filesystem>
#include <stack>
#include <string>
#include <unordered_map>

namespace fs = std::filesystem;

class FileSystemModel final : public Widgets::ITreeModel
{
public:
    FileSystemModel(const fs::path& rootPath)
    {
        buildModel(rootPath);
    }

    const std::vector<std::unique_ptr<Widgets::ModelItem>>& getRootItems() const noexcept override
    {
        return rootItems;
    }

    const Widgets::ModelItem* findById(const nbstl::Uuid& id) const noexcept override
    {
        auto it = uuidMap.find(id);
        return (it != uuidMap.end()) ? it->second : nullptr;
    }

    std::string data(const Widgets::ModelItem& item) const noexcept override
    {
        auto it = pathMap.find(item.getUuid());
        if (it != pathMap.end())
        {
            return it->second.filename().string();
        }
        return "Unknown Item";
    }

    void forEach(std::function<void(const Widgets::ModelItem&)> func) const noexcept override
    {
        for (auto const& [uuid, itemPtr] : uuidMap)
        {
            func(*itemPtr);
        }
    }

    size_t size() const noexcept override
    {
        return uuidMap.size();
    }

    fs::path getPath(const Widgets::ModelItem& item) const
    {
        auto it = pathMap.find(item.getUuid());
        return (it != pathMap.end()) ? it->second : fs::path();
    }

    void rebuildModel(const fs::path& rootPath)
    {
        pathMap.clear();
        uuidMap.clear();
        rootItems.clear();

        buildModel(rootPath);
    }

private:
    void buildModel(const fs::path& rootPath) noexcept
    {
        if (!fs::exists(rootPath) || !fs::is_directory(rootPath))
        {
            return;
        }

        auto rootItem = std::make_unique<Widgets::ModelItem>(nullptr, nullptr, 0);
        Widgets::ModelItem* rootPtr = rootItem.get();
        
        uuidMap[rootPtr->getUuid()] = rootPtr;
        pathMap[rootPtr->getUuid()] = rootPath;
        rootItems.push_back(std::move(rootItem));

        struct StackItem
        {
            fs::path path;
            Widgets::ModelItem* parentModelItem;
            size_t depth;
        };

        std::stack<StackItem> stk;
        stk.push({rootPath, rootPtr, 0});

        while (!stk.empty())
        {
            auto [currentPath, parentItem, depth] = stk.top();
            stk.pop();

            try
            {
                for (const auto& entry : fs::directory_iterator(currentPath))
                {
                    if(!entry.is_directory())
                    {
                        continue;
                    }
                    
                    auto childItem = std::make_unique<Widgets::ModelItem>(
                        nullptr, parentItem, depth + 1
                    );

                    Widgets::ModelItem* childPtr = childItem.get();
                    
                    uuidMap[childPtr->getUuid()] = childPtr;
                    pathMap[childPtr->getUuid()] = entry.path();
                    
                    parentItem->children.push_back(std::move(childItem));

                    if (entry.is_directory())
                    {
                        stk.push({entry.path(), childPtr, depth + 1});
                    }
                }
            }
            catch (const fs::filesystem_error& e)
            {
                continue;
            }
        }
    }

private:
    std::vector<std::unique_ptr<Widgets::ModelItem>> rootItems;
    
    std::unordered_map<nbstl::Uuid, Widgets::ModelItem*> uuidMap;
    std::unordered_map<nbstl::Uuid, fs::path> pathMap;
};

#endif