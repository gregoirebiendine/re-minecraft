#ifndef RE_MINECRAFT_ECSCOMPONENT_H
#define RE_MINECRAFT_ECSCOMPONENT_H

#include <cstdint>
#include <vector>
#include <algorithm>
#include <memory>
#include <limits>
#include <ranges>

#include "ECS/IEntity.h"

namespace ECS
{
    class IComponentPool
    {
        public:
            virtual ~IComponentPool() = default;
            virtual bool has(EntityId id) const = 0;
            virtual void remove(EntityId id) = 0;
            virtual void clear() = 0;
            virtual std::size_t size() const = 0;
    };

    template<typename T>
    class ComponentPool : public IComponentPool
    {
        SparseSet entitySet;
        std::vector<T> components;

        public:
            explicit ComponentPool(std::size_t max_entities = 0) :
                entitySet(max_entities)
            {}

            T& add(const EntityId id, const T& component)
            {
                if (entitySet.contains(id)) {
                    const Index index = entitySet.getIndex(id);
                    components[index] = component;
                    return components[index];
                }

                const Index index = entitySet.insert(id);

                if (index >= components.size())
                    components.resize(index + 1);

                components[index] = component;
                return components[index];
            }

            T& get(const EntityId id)
            {
                Index index = entitySet.getIndex(id);
                return components[index];
            }
            [[nodiscard]] const T& get(const EntityId id) const
            {
                Index index = entitySet.getIndex(id);
                return components[index];
            }
            T* tryGet(const EntityId id)
            {
                if (!entitySet.contains(id))
                    return nullptr;

                const Index index = entitySet.getIndex(id);
                return &components[index];
            }

            void remove(const EntityId id) override
            {
                if (!entitySet.contains(id))
                    return;

                Index removed_index = entitySet.getIndex(id);
                auto last_index = static_cast<Index>(entitySet.size() - 1);

                if (removed_index != last_index)
                    components[removed_index] = std::move(components[last_index]);

                entitySet.remove(id);
            }

            [[nodiscard]] bool has(const EntityId id) const override
            {
                return entitySet.contains(id);
            }

            template<typename Func>
            void forEach(Func&& callback)
            {
                const auto& dense = entitySet.getDense();

                for (std::size_t i = 0; i < entitySet.size(); ++i)
                    callback(dense[i], components[i]);
            }

            void clear() override
            {
                entitySet = SparseSet{};
                components.clear();
            }
            [[nodiscard]] std::size_t size() const override { return entitySet.size(); }

            [[nodiscard]] const SparseSet& getEntitySet() const { return entitySet; }
    };

    class ComponentTypeRegistry
    {
        inline static std::uint32_t nextTypeId = 0;

        public:
            template<typename T>
            static std::uint32_t getTypeId()
            {
                static std::uint32_t id = nextTypeId++;
                return id;
            }
    };

    template<typename T>
    class View
    {
        ComponentPool<T>& pool;

        public:
            explicit View(ComponentPool<T>& pool) :
                pool(pool)
            {}

            template<typename Func>
            void forEach(Func&& callback)
            {
                const auto& dense = pool.getEntitySet().getDense();

                for (std::size_t i = 0; i < pool.size(); ++i)
                    callback(dense[i], pool.get(dense[i]));
            }
    };

    template<typename T1, typename T2>
    class View2
    {
        ComponentPool<T1>& pool1;
        ComponentPool<T2>& pool2;

        public:
            View2(ComponentPool<T1>& pool1, ComponentPool<T2>& pool2) :
                pool1(pool1),
                pool2(pool2)
            {}

            template<typename Func>
            void forEach(Func&& callback)
            {
                if (pool1.size() <= pool2.size())
                {
                    const auto& dense = pool1.getEntitySet().getDense();
                    for (std::size_t i = 0; i < pool1.size(); ++i)
                    {
                        const EntityId id = dense[i];
                        if (pool2.has(id))
                            callback(id, pool1.get(id), pool2.get(id));
                    }
                }
                else
                {
                    const auto& dense = pool2.getEntitySet().getDense();
                    for (std::size_t i = 0; i < pool2.size(); ++i)
                    {
                        const EntityId id = dense[i];
                        if (pool1.has(id))
                            callback(id, pool1.get(id), pool2.get(id));
                    }
                }
            }
    };

    template<typename T1, typename T2, typename T3>
    class View3
    {
        ComponentPool<T1>& pool1;
        ComponentPool<T2>& pool2;
        ComponentPool<T3>& pool3;

        public:
            View3(ComponentPool<T1>& p1, ComponentPool<T2>& p2, ComponentPool<T3>& p3) :
                pool1(p1),
                pool2(p2),
                pool3(p3)
            {}

            template<typename Func>
            void forEach(Func&& callback)
            {
                // Find the smallest pool
                ComponentPool<T1>* smallestRef = &pool1;
                std::size_t smallestSize = pool1.size();

                if (pool2.size() < smallestSize) smallestSize = pool2.size();
                if (pool3.size() < smallestSize) smallestSize = pool3.size();

                // Iterate smallest pool, check others
                auto iterate = [&](auto& leadPool)
                {
                    const auto& dense = leadPool.getEntitySet().getDense();
                    for (std::size_t i = 0; i < leadPool.size(); ++i)
                    {
                        const EntityId id = dense[i];
                        if (pool1.has(id) && pool2.has(id) && pool3.has(id))
                            callback(id, pool1.get(id),
                                     pool2.get(id), pool3.get(id));
                    }
                };

                if (pool1.size() == smallestSize)
                    iterate(pool1);
                else if (pool2.size() == smallestSize)
                    iterate(pool2);
                else
                    iterate(pool3);
            }
    };
}

#endif //RE_MINECRAFT_ECSCOMPONENT_H