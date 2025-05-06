#ifndef SRC_ECS_ECS_HPP
#define SRC_ECS_ECS_HPP

#include <unordered_map>
#include <typeindex>
#include <vector>
#include <memory>
#include <functional>
#include "../Utils/Indexator.hpp"

namespace nb
{
    namespace Ecs
    {
        class Entity
        {
        public:

            template <typename T>
            void add(const T &component);

            template <typename T>
            T &get();

            bool operator==(const Entity &other) const { return id == other.id; }
            bool operator!=(const Entity &other) const { return !(*this == other); }

            size_t getId() const { return id; }

        private:
            friend class EcsManager;

            Entity(size_t id, class EcsManager *manager)
                : id(id), manager(manager) {}

            size_t id = 0;
            class EcsManager *manager = nullptr;
        };

        class EcsManager
        {
        private:
            class IComponentPool
            {
            public:
                virtual ~IComponentPool() = default;
                virtual std::vector<Entity> getEntitiesAsArray() const = 0;
            };

            template <class T>
            class ConcreteComponentPool : public IComponentPool
            {
            public:
                void add(const Entity &entity, const T &component)
                {
                    pool[entity] = component;
                }

                std::vector<Entity> getEntitiesAsArray() const override
                {
                    std::vector<Entity> entities;
                    entities.reserve(pool.size());
                    for (const auto &pair : pool)
                    {
                        entities.push_back(pair.first);
                    }
                    return entities;
                }

                T &get(const Entity &entity)
                {
                    return pool.at(entity);
                }

            private:
                std::unordered_map<Entity, T> pool;
            };

            std::unordered_map<std::type_index, std::unique_ptr<IComponentPool>> components;
            Utils::Indexator indexator;

        public:
            Entity createEntity()
            {
                return Entity(indexator.index(), this);
            }

            template <typename T>
            void addComponent(const Entity &entity, const T &component)
            {
                std::type_index type(typeid(T));
                if (components.find(type) == components.end())
                {
                    components[type] = std::make_unique<ConcreteComponentPool<T>>();
                }
                auto &pool = static_cast<ConcreteComponentPool<T> &>(*components[type]);
                pool.add(entity, component);
            }

            template <typename T>
            T &getComponent(const Entity &entity)
            {
                std::type_index type(typeid(T));
                auto &pool = static_cast<ConcreteComponentPool<T> &>(*components.at(type));
                return pool.get(entity);
            }

            template <typename T>
            std::vector<Entity> getEntitiesWithComponent()
            {
                std::type_index type(typeid(T));
                return components.at(type)->getEntitiesAsArray();
            }
        };

        template <typename T>
        void Entity::add(const T &component)
        {
            manager->addComponent(*this, component);
        }

        template <typename T>
        T &Entity::get()
        {
            return manager->getComponent<T>(*this);
        }

        class ISystem
        {
        public:
            virtual ~ISystem() = default;
        };

    };
};

// Специализация std::hash для Entity
namespace std
{
    template <>
    struct hash<nb::Ecs::Entity>
    {
        size_t operator()(const nb::Ecs::Entity &entity) const noexcept
        {
            return hash<size_t>{}(entity.getId());
        }
    };
};

#endif