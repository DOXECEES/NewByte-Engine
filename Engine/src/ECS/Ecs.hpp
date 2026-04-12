#ifndef SRC_ECS_ECS_HPP
#define SRC_ECS_ECS_HPP

#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include <cstdint>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <Reflection/Reflection.hpp>

namespace nb::Ecs
{
    using EntityID = uint32_t;
    using ComponentTypeID = uint32_t;

    struct Entity
    {
        EntityID id = 0;

        bool operator==(const Entity& other) const noexcept
        {
            return id == other.id;
        }

        bool operator!=(const Entity& other) const noexcept
        {
            return !(*this == other);
        }
    };

    inline ComponentTypeID generateComponentTypeID()
    {
        static ComponentTypeID counter = 0;
        return counter++;
    }

    template <typename T> struct ComponentType
    {
        static ComponentTypeID id() noexcept
        {
            static ComponentTypeID value = generateComponentTypeID();
            return value;
        }
    };

    template <typename T> class ComponentStorage
    {
    public:

        void
        add(EntityID entity,
            const T& component)
        {
            if constexpr (std::is_copy_assignable_v<T>)
            {
                if (indexMap.contains(entity))
                {
                    data[indexMap[entity]] = component;
                    return;
                }

                indexMap[entity] = data.size();
                entities.push_back(entity);
                data.push_back(component);
            }
            else
            {
            }
        }



        void
        add(EntityID entity,
            T&& component)
        {
            if (indexMap.contains(entity))
            {
                data[indexMap[entity]] = std::move(component);
                return; 
            }

            indexMap[entity] = data.size();

            entities.push_back(entity);
            data.push_back(std::move(component));
        }

        void remove(EntityID entity)
        {
            auto it = indexMap.find(entity);
            if (it == indexMap.end())
            {
                return;
            }

            size_t index = it->second;
            size_t last = data.size() - 1;

            entities[index] = entities[last];
            data[index]     = std::move(data[last]);

            indexMap[entities[index]] = index;

            entities.pop_back();
            data.pop_back();
            indexMap.erase(it);
        }

        bool contains(EntityID entity) const
        {
            return indexMap.contains(entity);
        }

        T& get(EntityID entity)
        {
            return data.at(indexMap.at(entity));
        }

        const std::vector<T>& components() const noexcept
        {
            return data;
        }

        const std::vector<EntityID>& entitiesView() const noexcept
        {
            return entities;
        }

        void clear()
        {
            indexMap.clear();
            entities.clear();
            data.clear();
        }


    private:
        std::unordered_map<EntityID, size_t> indexMap;

        std::vector<EntityID> entities;
        std::vector<T> data;
    };

    struct StorageWrapperBase
    {
        virtual ~StorageWrapperBase() = default;
        virtual void remove(EntityID entity) = 0;

        // --- НОВЫЕ МЕТОДЫ ДЛЯ АВТО-ИНСПЕКЦИИ ---
        virtual bool contains(EntityID entity) const = 0;
        virtual void* getRaw(EntityID entity) = 0;
        virtual Reflect::TypeInfo* getTypeInfo() = 0;

        virtual void addRaw(
            EntityID entity,
            void* component
        ) = 0;

        virtual void addDefault(EntityID entity) = 0; 
        virtual void clear()                     = 0;

    };

    template <typename T> struct StorageWrapper : StorageWrapperBase
    {
        ComponentStorage<T> storage;

        void remove(EntityID entity) override
        {
            storage.remove(entity);
        }

        void clear() override
        {
            storage.clear();
        }

        // Реализация новых методов
        bool contains(EntityID entity) const override
        {
            return storage.contains(entity);
        }

        void* getRaw(EntityID entity) override
        {
            return &storage.get(entity);
        }

        Reflect::TypeInfo* getTypeInfo() override
        {
            return Reflect::getType<T>();
        }

        void addRaw(
            EntityID entity,
            void*    component
        ) override
        {
            storage.add(entity, std::move(*static_cast<T*>(component)));
        }


        void addDefault(EntityID entity) override
        {
            storage.add(entity, T{});
        }
    };

    class ECSRegistry
    {
    public:
        Entity createEntity()
        {
            return Entity{nextEntityID++};
        }

        void clear()
        {
            for (auto& storage : storages)
            {
                if (storage)
                {
                    storage->clear();
                }
            }
            nextEntityID = 1;
        }


        void destroyEntity(Entity entity)
        {
            for (auto& storage : storages)
            {
                if (storage)
                {
                    storage->remove(entity.id);
                }
            }
        }

        template <typename T>
        void
        add(Entity entity,
            T&&    component) 
        {
            using ComponentType = std::decay_t<T>;
            getStorage<ComponentType>().add(entity.id, std::forward<T>(component));
        }


        template <typename T> T& get(Entity entity)
        {
            return getStorage<T>().get(entity.id);
        }

        template <typename T> bool has(Entity entity)
        {
            return getStorage<T>().contains(entity.id);
        }

        /// --------------------------------------------------
        /// Component Type Metadata
        /// --------------------------------------------------

        std::vector<ComponentTypeID> getComponentTypes() const noexcept
        {
            return componentTypes;
        }

        const std::vector<std::unique_ptr<StorageWrapperBase>>& getAllStorages() const
        {
            return storages;
        }

    private:
        void registerComponentType(ComponentTypeID id)
        {
            if (std::find(componentTypes.begin(), componentTypes.end(), id) == componentTypes.end())
            {
                componentTypes.push_back(id);
            }
        }

    public:
        template <typename T>
        ComponentStorage<std::decay_t<T>>& getStorage()
        {
            using PureT = std::decay_t<T>;

            ComponentTypeID id = ComponentType<PureT>::id();

            if (storages.size() <= id)
            {
                storages.resize(id + 1);
            }

            if (!storages[id])
            {
                storages[id] = std::make_unique<StorageWrapper<PureT>>();
                registerComponentType(id);
            }

            return static_cast<StorageWrapper<PureT>*>(storages[id].get())->storage;
        }


        template <typename T> const ComponentStorage<T>& getStorage() const
        {
            return const_cast<ECSRegistry*>(this)->getStorage<T>();
        }

    private:
        EntityID nextEntityID = 1;

        std::vector<std::unique_ptr<StorageWrapperBase>> storages;

        std::vector<ComponentTypeID> componentTypes;
    };

}; 

#endif