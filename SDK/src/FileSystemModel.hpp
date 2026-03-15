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

    // Возвращает имя файла или папки для отображения в TreeView
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

    // Полезный метод для получения полного пути из айтема
    fs::path getPath(const Widgets::ModelItem& item) const
    {
        auto it = pathMap.find(item.getUuid());
        return (it != pathMap.end()) ? it->second : fs::path();
    }

private:
    void buildModel(const fs::path& rootPath) noexcept
    {
        if (!fs::exists(rootPath) || !fs::is_directory(rootPath))
        {
            return;
        }

        // 1. Создаем корневой элемент
        auto rootItem = std::make_unique<Widgets::ModelItem>(nullptr, nullptr, 0);
        Widgets::ModelItem* rootPtr = rootItem.get();
        
        uuidMap[rootPtr->getUuid()] = rootPtr;
        pathMap[rootPtr->getUuid()] = rootPath;
        rootItems.push_back(std::move(rootItem));

        // 2. Рекурсивный обход через стек (чтобы избежать переполнения при глубоких путях)
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
                    
                    // Регистрируем айтем
                    uuidMap[childPtr->getUuid()] = childPtr;
                    pathMap[childPtr->getUuid()] = entry.path();
                    
                    parentItem->children.push_back(std::move(childItem));

                    // Если это папка — идем глубже
                    if (entry.is_directory())
                    {
                        stk.push({entry.path(), childPtr, depth + 1});
                    }
                }
            }
            catch (const fs::filesystem_error& e)
            {
                // Логируем ошибку доступа, если нужно
                continue;
            }
        }
    }

private:
    std::vector<std::unique_ptr<Widgets::ModelItem>> rootItems;
    
    // Карта для быстрого поиска айтема по ID (нужна для ITreeModel)
    std::unordered_map<nbstl::Uuid, Widgets::ModelItem*> uuidMap;
    
    // Карта соответствия: UUID айтема -> Путь в файловой системе
    std::unordered_map<nbstl::Uuid, fs::path> pathMap;
};

#endif