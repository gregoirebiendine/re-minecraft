#ifndef RE_MINECRAFT_ECSSYSTEM_H
#define RE_MINECRAFT_ECSSYSTEM_H

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <memory>
#include <limits>
#include <ranges>
#include <stdexcept>

#include "ECS/Entity.h"
#include "ECS/Component.h"

namespace ECS
{
    class Handler
    {
        EntityManager entityManager;
        std::unordered_map<std::uint32_t, std::unique_ptr<IComponentPool>> componentPools;
        std::uint32_t maxEntities;

        public:
            explicit Handler(const std::uint32_t max = 10000) :
                maxEntities(max)
            {}

            template<typename T>
            ComponentPool<T>& getPool()
            {
                std::uint32_t typeId = ComponentTypeRegistry::getTypeId<T>();

                const auto it = componentPools.find(typeId);
                if (it == componentPools.end())
                {
                    auto pool = std::make_unique<ComponentPool<T>>(maxEntities);
                    auto* ptr = pool.get();
                    componentPools[typeId] = std::move(pool);
                    return *static_cast<ComponentPool<T>*>(ptr);
                }

                return *static_cast<ComponentPool<T>*>(it->second.get());
            }

            Entity createEntity()
            {
                return entityManager.create();
            }

            void destroyEntity(const Entity entity)
            {
                if (!entityManager.isAlive(entity))
                    return;

                for (const auto& pool : componentPools | std::views::values)
                    pool->remove(entity.id);

                entityManager.destroy(entity);
            }

            template<typename T>
            T& addComponent(Entity entity, const T& component)
            {
                auto& pool = getPool<T>();

                return pool.add(entity.id, component);
            }

            template<typename T>
            void removeComponent(const Entity entity)
            {
                auto& pool = getPool<T>();
                pool.remove(entity.id);
            }

            template<typename T>
            T& getComponent(const Entity entity)
            {
                auto& pool = getPool<T>();
                return pool.get(entity.id);
            }

            template<typename T>
            bool hasComponent(const Entity entity) const
            {
                const std::uint32_t type_id = ComponentTypeRegistry::getTypeId<T>();

                const auto it = componentPools.find(type_id);
                if (it == componentPools.end())
                    return false;

                return it->second->has(entity.id);
            }

            template<typename T>
            View<T> query()
            {
                return View<T>(getPool<T>());
            }

            template<typename T1, typename T2>
            View2<T1, T2> query()
            {
                return View2<T1, T2>(getPool<T1>(), getPool<T2>());
            }

            template<typename T1, typename T2, typename T3>
            View3<T1, T2, T3> query()
            {
                return View3<T1, T2, T3>(getPool<T1>(), getPool<T2>(), getPool<T3>());
            }
    };

    class ISystem
    {
        public:
            virtual ~ISystem() = default;
            virtual void update(Handler& handler, float deltaTime) = 0;
    };

    class IRenderSystem
    {
        public:
            virtual ~IRenderSystem() = default;
            virtual void render(Handler& handler) = 0;
    };

    class SystemScheduler
    {
        std::vector<std::unique_ptr<ISystem>> systems;
        std::vector<std::unique_ptr<IRenderSystem>> renderSystems;

        public:
            template<typename T, typename... Args>
            T& registerSystem(Args&&... args)
            {
                auto system = std::make_unique<T>(std::forward<Args>(args)...);
                auto* ptr = system.get();

                if constexpr (std::is_base_of_v<ISystem, T>)
                    systems.push_back(std::move(system));
                else if constexpr (std::is_base_of_v<IRenderSystem, T>)
                    renderSystems.push_back(std::move(system));

                return *ptr;
            }

            template<typename T>
            T& getSystem()
            {
                for (const auto& system : systems)
                    if (auto* ptr = dynamic_cast<T*>(system.get()))
                        return *ptr;

                for (const auto& system : renderSystems)
                    if (auto* ptr = dynamic_cast<T*>(system.get()))
                        return *ptr;

                throw std::runtime_error("[Farfield::ECS] System not registered");
            }

            void update(Handler& handler, const float dt) const
            {
                for (const auto& system : systems)
                    system->update(handler, dt);
            }

            void render(Handler& handler) const
            {
                for (const auto& system : renderSystems)
                    system->render(handler);
            }
    };
}

#endif //RE_MINECRAFT_ECSSYSTEM_H