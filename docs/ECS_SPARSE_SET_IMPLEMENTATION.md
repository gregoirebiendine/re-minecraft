# ECS Implementation Guide: Sparse Set Approach

A step-by-step guide to implementing an Entity Component System using the Sparse Set data structure.

---

## Table of Contents

1. [Understanding Sparse Sets](#1-understanding-sparse-sets)
2. [Step 1: Implement the Sparse Set Data Structure](#step-1-implement-the-sparse-set-data-structure)
3. [Step 2: Create the Entity Manager](#step-2-create-the-entity-manager)
4. [Step 3: Build the Component Storage](#step-3-build-the-component-storage)
5. [Step 4: Implement the World/Registry](#step-4-implement-the-worldregistry)
6. [Step 5: Create the View/Query System](#step-5-create-the-viewquery-system)
7. [Step 6: Implement Systems](#step-6-implement-systems)
8. [Complete Example](#complete-example)
9. [Optimization Considerations](#optimization-considerations)

---

## 1. Understanding Sparse Sets

### What is a Sparse Set?

A Sparse Set is a data structure that provides:
- **O(1)** insertion
- **O(1)** deletion
- **O(1)** lookup (check if element exists)
- **O(n)** iteration (where n = number of elements, not capacity)

It uses two arrays:
- **Sparse Array**: Maps entity IDs to indices in the dense array (size = max entity ID)
- **Dense Array**: Stores actual entity IDs contiguously (size = number of stored entities)

### Visual Representation

```
Entities with component: [2, 5, 8, 12]

Sparse Array (indexed by entity ID):
Index:   0    1    2    3    4    5    6    7    8    9   10   11   12
Value: [INV][INV][ 0 ][INV][INV][ 1 ][INV][INV][ 2 ][INV][INV][INV][ 3 ]
                  ^              ^              ^                    ^
                  |              |              |                    |
                Points to dense index where this entity's data lives

Dense Array (contiguous storage):
Index:   0    1    2    3
Value: [ 2 ][ 5 ][ 8 ][12]  <- Entity IDs
Data:  [D2 ][D5 ][D8 ][D12] <- Component data (parallel array)
```

### Why Sparse Sets for ECS?

1. **Fast component access**: O(1) to get/check component for any entity
2. **Fast iteration**: Dense array allows cache-friendly sequential access
3. **Fast add/remove**: O(1) to add or remove components from entities
4. **Memory trade-off**: Uses extra memory for sparse array but worth it for performance

---

## Step 1: Implement the Sparse Set Data Structure

### 1.1 Define Constants and Types

```cpp
#include <cstdint>
#include <vector>
#include <limits>

namespace ECS
{
    using EntityId = std::uint32_t;
    using Index = std::uint32_t;

    inline static constexpr Index INVALID_INDEX = std::numeric_limits<Index>::max();
    inline static constexpr EntityId INVALID_ENTITY_ID = std::numeric_limits<EntityId>::max();
```

### 1.2 Basic Sparse Set Structure

```cpp
    class SparseSet
    {
        std::vector<Index> sparse;
        std::vector<EntityId> dense;
        std::size_t count = 0;

    public:
        explicit SparseSet(std::size_t max_entities = 0)
            : sparse(max_entities, INVALID_INDEX)
            , dense(max_entities)
        {
        }
```

### 1.3 Implement Contains (Check if Entity Exists)

```cpp
        bool contains(EntityId entity_id) const
        {
            if (entity_id >= sparse.size())
                return false;

            Index dense_index = sparse[entity_id];

            if (dense_index == INVALID_INDEX)
                return false;

            if (dense_index >= count)
                return false;

            return dense[dense_index] == entity_id;
        }
```

### 1.4 Implement Insert

```cpp
        Index insert(EntityId entity_id)
        {
            if (entity_id >= sparse.size())
                sparse.resize(entity_id + 1, INVALID_INDEX);

            if (contains(entity_id))
                return sparse[entity_id];

            Index new_index = static_cast<Index>(count);

            if (new_index >= dense.size())
                dense.resize(new_index + 1);

            dense[new_index] = entity_id;
            sparse[entity_id] = new_index;
            ++count;

            return new_index;
        }
```

### 1.5 Implement Remove (Swap-and-Pop)

This is the key trick: swap the removed element with the last element, then pop.

```cpp
        bool remove(EntityId entity_id)
        {
            if (!contains(entity_id))
                return false;

            Index removed_index = sparse[entity_id];
            Index last_index = static_cast<Index>(count - 1);
            EntityId last_entity = dense[last_index];

            // Swap last element into the removed slot
            dense[removed_index] = last_entity;
            sparse[last_entity] = removed_index;

            // Mark removed entity as invalid
            sparse[entity_id] = INVALID_INDEX;

            --count;

            return true;
        }
```

### 1.6 Implement Get Index

```cpp
        Index get_index(EntityId entity_id) const
        {
            if (!contains(entity_id))
                return INVALID_INDEX;

            return sparse[entity_id];
        }

        std::size_t size() const { return count; }

        const std::vector<EntityId>& get_dense() const { return dense; }
    };
```

---

## Step 2: Create the Entity Manager

### 2.1 Entity Structure with Generations

Generations help detect stale entity references (e.g., accessing a destroyed entity).

```cpp
    struct Entity
    {
        std::uint32_t id = INVALID_ENTITY_ID;
        std::uint32_t generation = 0;
    };

    inline Entity make_entity(std::uint32_t id, std::uint32_t generation)
    {
        return Entity{ id, generation };
    }

    inline static constexpr Entity NULL_ENTITY = { INVALID_ENTITY_ID, 0 };
```

### 2.2 Entity Manager Class

```cpp
    class EntityManager
    {
        std::vector<std::uint32_t> generations;
        std::vector<std::uint32_t> free_list;
        std::uint32_t next_id = 0;

    public:
        EntityManager() = default;
```

### 2.3 Create Entity

```cpp
        Entity create()
        {
            if (!free_list.empty())
            {
                std::uint32_t id = free_list.back();
                free_list.pop_back();
                return make_entity(id, generations[id]);
            }

            std::uint32_t id = next_id++;
            generations.push_back(0);
            return make_entity(id, 0);
        }
```

### 2.4 Destroy Entity

```cpp
        bool destroy(Entity entity)
        {
            if (!is_alive(entity))
                return false;

            ++generations[entity.id];
            free_list.push_back(entity.id);

            return true;
        }
```

### 2.5 Check if Entity is Alive

```cpp
        bool is_alive(Entity entity) const
        {
            if (entity.id >= generations.size())
                return false;

            return generations[entity.id] == entity.generation;
        }
    };
```

---

## Step 3: Build the Component Storage

### 3.1 Component Pool Interface

```cpp
    class IComponentPool
    {
    public:
        virtual ~IComponentPool() = default;
        virtual bool has(EntityId entity_id) const = 0;
        virtual void remove(EntityId entity_id) = 0;
        virtual void clear() = 0;
        virtual std::size_t size() const = 0;
    };
```

### 3.2 Typed Component Pool

```cpp
    template<typename T>
    class ComponentPool : public IComponentPool
    {
        SparseSet entity_set;
        std::vector<T> components;

    public:
        explicit ComponentPool(std::size_t max_entities = 0)
            : entity_set(max_entities)
        {
        }
```

### 3.3 Add Component

```cpp
        T& add(EntityId entity_id, const T& component)
        {
            if (entity_set.contains(entity_id))
            {
                Index index = entity_set.get_index(entity_id);
                components[index] = component;
                return components[index];
            }

            Index index = entity_set.insert(entity_id);

            if (index >= components.size())
                components.resize(index + 1);

            components[index] = component;
            return components[index];
        }
```

### 3.4 Get Component

```cpp
        T& get(EntityId entity_id)
        {
            Index index = entity_set.get_index(entity_id);
            return components[index];
        }

        const T& get(EntityId entity_id) const
        {
            Index index = entity_set.get_index(entity_id);
            return components[index];
        }
```

### 3.5 Try Get Component (Safe Version)

```cpp
        T* try_get(EntityId entity_id)
        {
            if (!entity_set.contains(entity_id))
                return nullptr;

            Index index = entity_set.get_index(entity_id);
            return &components[index];
        }
```

### 3.6 Remove Component (Swap-and-Pop)

```cpp
        void remove(EntityId entity_id) override
        {
            if (!entity_set.contains(entity_id))
                return;

            Index removed_index = entity_set.get_index(entity_id);
            Index last_index = static_cast<Index>(entity_set.size() - 1);

            if (removed_index != last_index)
                components[removed_index] = std::move(components[last_index]);

            entity_set.remove(entity_id);
        }
```

### 3.7 Has Component

```cpp
        bool has(EntityId entity_id) const override
        {
            return entity_set.contains(entity_id);
        }
```

### 3.8 Iterate Over All Components

```cpp
        template<typename Func>
        void for_each(Func&& callback)
        {
            const auto& dense = entity_set.get_dense();
            for (std::size_t i = 0; i < entity_set.size(); ++i)
            {
                callback(dense[i], components[i]);
            }
        }

        void clear() override
        {
            entity_set = SparseSet{};
            components.clear();
        }

        std::size_t size() const override { return entity_set.size(); }

        const SparseSet& get_entity_set() const { return entity_set; }
    };
```

---

## Step 4: Implement the World/Registry

### 4.1 Component Type Registration

```cpp
    class ComponentTypeRegistry
    {
        inline static std::uint32_t next_type_id = 0;

    public:
        template<typename T>
        static std::uint32_t get_type_id()
        {
            static std::uint32_t id = next_type_id++;
            return id;
        }
    };
```

### 4.2 World Class

```cpp
    #include <unordered_map>

    class World
    {
        EntityManager entity_manager;
        std::unordered_map<std::uint32_t, std::unique_ptr<IComponentPool>> component_pools;
        std::uint32_t max_entities;

    public:
        explicit World(std::uint32_t max_entities = 10000)
            : max_entities(max_entities)
        {
        }
```

### 4.3 Get or Create Component Pool

```cpp
        template<typename T>
        ComponentPool<T>& get_pool()
        {
            std::uint32_t type_id = ComponentTypeRegistry::get_type_id<T>();

            auto it = component_pools.find(type_id);
            if (it == component_pools.end())
            {
                auto pool = std::make_unique<ComponentPool<T>>(max_entities);
                auto* ptr = pool.get();
                component_pools[type_id] = std::move(pool);
                return *static_cast<ComponentPool<T>*>(ptr);
            }

            return *static_cast<ComponentPool<T>*>(it->second.get());
        }
```

### 4.4 Create Entity

```cpp
        Entity create_entity()
        {
            return entity_manager.create();
        }
```

### 4.5 Destroy Entity

```cpp
        void destroy_entity(Entity entity)
        {
            if (!entity_manager.is_alive(entity))
                return;

            for (auto& [type_id, pool] : component_pools)
                pool->remove(entity.id);

            entity_manager.destroy(entity);
        }
```

### 4.6 Add Component

```cpp
        template<typename T>
        T& add_component(Entity entity, const T& component)
        {
            auto& pool = get_pool<T>();
            return pool.add(entity.id, component);
        }
```

### 4.7 Remove Component

```cpp
        template<typename T>
        void remove_component(Entity entity)
        {
            auto& pool = get_pool<T>();
            pool.remove(entity.id);
        }
```

### 4.8 Get Component

```cpp
        template<typename T>
        T& get_component(Entity entity)
        {
            auto& pool = get_pool<T>();
            return pool.get(entity.id);
        }
```

### 4.9 Has Component

```cpp
        template<typename T>
        bool has_component(Entity entity) const
        {
            std::uint32_t type_id = ComponentTypeRegistry::get_type_id<T>();

            auto it = component_pools.find(type_id);
            if (it == component_pools.end())
                return false;

            return it->second->has(entity.id);
        }
```

---

## Step 5: Create the View/Query System

Views allow efficient iteration over entities with specific component combinations.

### 5.1 Single Component View

```cpp
        template<typename T>
        class View
        {
            ComponentPool<T>& pool;

        public:
            explicit View(ComponentPool<T>& pool)
                : pool(pool)
            {
            }

            template<typename Func>
            void for_each(Func&& callback)
            {
                const auto& dense = pool.get_entity_set().get_dense();
                for (std::size_t i = 0; i < pool.size(); ++i)
                {
                    callback(dense[i], pool.get(dense[i]));
                }
            }
        };
```

### 5.2 Multi-Component View (Two Components)

For multiple components, iterate over the **smallest** pool and check the other.

```cpp
        template<typename T1, typename T2>
        class View2
        {
            ComponentPool<T1>& pool1;
            ComponentPool<T2>& pool2;

        public:
            View2(ComponentPool<T1>& pool1, ComponentPool<T2>& pool2)
                : pool1(pool1), pool2(pool2)
            {
            }

            template<typename Func>
            void for_each(Func&& callback)
            {
                if (pool1.size() <= pool2.size())
                {
                    const auto& dense = pool1.get_entity_set().get_dense();
                    for (std::size_t i = 0; i < pool1.size(); ++i)
                    {
                        EntityId entity_id = dense[i];
                        if (pool2.has(entity_id))
                            callback(entity_id, pool1.get(entity_id), pool2.get(entity_id));
                    }
                }
                else
                {
                    const auto& dense = pool2.get_entity_set().get_dense();
                    for (std::size_t i = 0; i < pool2.size(); ++i)
                    {
                        EntityId entity_id = dense[i];
                        if (pool1.has(entity_id))
                            callback(entity_id, pool1.get(entity_id), pool2.get(entity_id));
                    }
                }
            }
        };
```

### 5.3 Multi-Component View (Three Components)

```cpp
        template<typename T1, typename T2, typename T3>
        class View3
        {
            ComponentPool<T1>& pool1;
            ComponentPool<T2>& pool2;
            ComponentPool<T3>& pool3;

        public:
            View3(ComponentPool<T1>& p1, ComponentPool<T2>& p2, ComponentPool<T3>& p3)
                : pool1(p1), pool2(p2), pool3(p3)
            {
            }

            template<typename Func>
            void for_each(Func&& callback)
            {
                // Find the smallest pool
                ComponentPool<T1>* smallest_ref = &pool1;
                std::size_t smallest_size = pool1.size();

                if (pool2.size() < smallest_size) smallest_size = pool2.size();
                if (pool3.size() < smallest_size) smallest_size = pool3.size();

                // Iterate smallest pool, check others
                auto iterate = [&](auto& lead_pool)
                {
                    const auto& dense = lead_pool.get_entity_set().get_dense();
                    for (std::size_t i = 0; i < lead_pool.size(); ++i)
                    {
                        EntityId entity_id = dense[i];
                        if (pool1.has(entity_id) && pool2.has(entity_id) && pool3.has(entity_id))
                            callback(entity_id, pool1.get(entity_id),
                                     pool2.get(entity_id), pool3.get(entity_id));
                    }
                };

                if (pool1.size() == smallest_size)
                    iterate(pool1);
                else if (pool2.size() == smallest_size)
                    iterate(pool2);
                else
                    iterate(pool3);
            }
        };
```

### 5.4 Add Query Methods to World

```cpp
        // In World class
        template<typename T>
        View<T> query()
        {
            return View<T>(get_pool<T>());
        }

        template<typename T1, typename T2>
        View2<T1, T2> query()
        {
            return View2<T1, T2>(get_pool<T1>(), get_pool<T2>());
        }

        template<typename T1, typename T2, typename T3>
        View3<T1, T2, T3> query()
        {
            return View3<T1, T2, T3>(get_pool<T1>(), get_pool<T2>(), get_pool<T3>());
        }
    };
```

---

## Step 6: Implement Systems

### 6.1 System Interface

```cpp
    class ISystem
    {
    public:
        virtual ~ISystem() = default;
        virtual void update(World& world, float delta_time) = 0;
    };
```

### 6.2 Example: Movement System

```cpp
    struct Position
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    };

    struct Velocity
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    };

    class MovementSystem : public ISystem
    {
    public:
        void update(World& world, float dt) override
        {
            auto view = world.query<Position, Velocity>();

            view.for_each([dt](EntityId id, Position& pos, Velocity& vel)
            {
                pos.x += vel.x * dt;
                pos.y += vel.y * dt;
                pos.z += vel.z * dt;
            });
        }
    };
```

### 6.3 Example: Gravity System

```cpp
    struct Gravity
    {
        float strength = 9.81f;
        float terminal_velocity = 50.0f;
    };

    struct Grounded
    {
        bool is_grounded = false;
    };

    class GravitySystem : public ISystem
    {
    public:
        void update(World& world, float dt) override
        {
            auto view = world.query<Velocity, Gravity, Grounded>();

            view.for_each([dt](EntityId id, Velocity& vel, Gravity& grav, Grounded& grounded)
            {
                if (!grounded.is_grounded)
                {
                    vel.y -= grav.strength * dt;

                    if (vel.y < -grav.terminal_velocity)
                        vel.y = -grav.terminal_velocity;
                }
            });
        }
    };
```

### 6.4 Example: Lifetime System (With Entity Destruction)

```cpp
    struct Lifetime
    {
        float remaining = 0.0f;
    };

    class LifetimeSystem : public ISystem
    {
    public:
        void update(World& world, float dt) override
        {
            // Collect entities to destroy (can't destroy during iteration)
            std::vector<Entity> to_destroy;

            auto view = world.query<Lifetime>();

            view.for_each([&](EntityId id, Lifetime& lifetime)
            {
                lifetime.remaining -= dt;

                if (lifetime.remaining <= 0.0f)
                    to_destroy.push_back(Entity{ id, 0 }); // generation lookup needed in practice
            });

            for (auto& entity : to_destroy)
                world.destroy_entity(entity);
        }
    };
```

### 6.5 System Scheduler

```cpp
    class SystemScheduler
    {
        std::vector<std::unique_ptr<ISystem>> systems;

    public:
        template<typename T, typename... Args>
        void register_system(Args&&... args)
        {
            systems.push_back(std::make_unique<T>(std::forward<Args>(args)...));
        }

        void update(World& world, float dt)
        {
            for (auto& system : systems)
                system->update(world, dt);
        }
    };

} // namespace ECS
```

---

## Complete Example

### Full Working Example

```cpp
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <limits>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <cstdio>

// (Assumes all ECS classes above are included via ECS.h)

// ==========================================
// COMPONENTS
// ==========================================

struct Position
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct Velocity
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct Gravity
{
    float strength = 9.81f;
    float terminal_velocity = 50.0f;
};

struct Health
{
    int current = 0;
    int maximum = 0;
};

struct PlayerTag {};

// ==========================================
// SYSTEMS
// ==========================================

class PhysicsSystem : public ECS::ISystem
{
public:
    void update(ECS::World& world, float dt) override
    {
        // Apply gravity to all entities with Velocity and Gravity
        auto gravity_view = world.query<Velocity, Gravity>();
        gravity_view.for_each([dt](ECS::EntityId id, Velocity& vel, Gravity& grav)
        {
            vel.y -= grav.strength * dt;
            vel.y = std::max(vel.y, -grav.terminal_velocity);
        });

        // Apply velocity to position
        auto movement_view = world.query<Position, Velocity>();
        movement_view.for_each([dt](ECS::EntityId id, Position& pos, Velocity& vel)
        {
            pos.x += vel.x * dt;
            pos.y += vel.y * dt;
            pos.z += vel.z * dt;
        });
    }
};

class HealthSystem : public ECS::ISystem
{
public:
    void update(ECS::World& world, float dt) override
    {
        std::vector<ECS::Entity> to_destroy;

        auto health_view = world.query<Health>();
        health_view.for_each([&](ECS::EntityId id, Health& health)
        {
            if (health.current <= 0)
                to_destroy.push_back(ECS::Entity{ id, 0 });
        });

        for (auto& entity : to_destroy)
            world.destroy_entity(entity);
    }
};

// ==========================================
// MAIN GAME LOOP
// ==========================================

int main()
{
    // Create world
    ECS::World world(10000);

    // Create systems
    ECS::SystemScheduler scheduler;
    scheduler.register_system<PhysicsSystem>();
    scheduler.register_system<HealthSystem>();

    // Create player entity
    ECS::Entity player = world.create_entity();
    world.add_component(player, Position{ 0.0f, 100.0f, 0.0f });
    world.add_component(player, Velocity{ 0.0f, 0.0f, 0.0f });
    world.add_component(player, Gravity{ 15.0f, 50.0f });
    world.add_component(player, Health{ 20, 20 });
    world.add_component(player, PlayerTag{});

    // Create some falling objects
    for (int i = 0; i < 100; ++i)
    {
        ECS::Entity entity = world.create_entity();
        world.add_component(entity, Position{
            static_cast<float>(std::rand() % 100 - 50),
            static_cast<float>(std::rand() % 100 + 50),
            static_cast<float>(std::rand() % 100 - 50)
        });
        world.add_component(entity, Velocity{ 0.0f, 0.0f, 0.0f });
        world.add_component(entity, Gravity{ 9.81f, 30.0f });
    }

    // Game loop
    auto last_time = std::chrono::steady_clock::now();
    bool game_running = true;

    while (game_running)
    {
        auto current_time = std::chrono::steady_clock::now();
        float delta_time = std::chrono::duration<float>(current_time - last_time).count();
        last_time = current_time;

        // Update all systems
        scheduler.update(world, delta_time);

        // Cap frame rate (~60 FPS)
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    return 0;
}
```

---

## Optimization Considerations

### 1. Pool Sorting for Better Cache Performance

Sort component pools so entities accessed together are stored contiguously.

```cpp
template<typename T>
void ComponentPool<T>::sort_by_entity_id()
{
    const auto& dense = entity_set.get_dense();
    std::size_t n = entity_set.size();

    // Create index mapping
    std::vector<std::size_t> indices(n);
    std::iota(indices.begin(), indices.end(), 0);

    // Sort indices by entity ID
    std::sort(indices.begin(), indices.end(), [&](std::size_t a, std::size_t b)
    {
        return dense[a] < dense[b];
    });

    // Apply permutation to dense array and components
    std::vector<EntityId> sorted_dense(n);
    std::vector<T> sorted_components(n);

    for (std::size_t i = 0; i < n; ++i)
    {
        sorted_dense[i] = dense[indices[i]];
        sorted_components[i] = std::move(components[indices[i]]);
    }

    // Rebuild (would need mutable access to entity_set internals)
    // This is a conceptual example; actual implementation depends on SparseSet API
}
```

### 2. Group Entities by Archetype (Hybrid Approach)

Track which components each entity has for faster queries.

```cpp
#include <bitset>

constexpr std::size_t MAX_COMPONENT_TYPES = 64;

struct Archetype
{
    std::bitset<MAX_COMPONENT_TYPES> component_mask;
    std::vector<EntityId> entities;
};

class ArchetypeIndex
{
    std::unordered_map<std::bitset<MAX_COMPONENT_TYPES>, Archetype> archetypes;
    std::unordered_map<EntityId, std::bitset<MAX_COMPONENT_TYPES>> entity_archetype;

    // When querying, first filter archetypes by component mask,
    // then only iterate matching archetypes
};
```

### 3. Deferred Entity Destruction

Batch entity destruction to avoid issues during iteration.

```cpp
class World
{
    // ... existing members ...
    std::vector<Entity> pending_destroy;

public:
    void destroy_entity_deferred(Entity entity)
    {
        pending_destroy.push_back(entity);
    }

    void flush_pending_destroys()
    {
        for (auto& entity : pending_destroy)
            destroy_entity(entity);

        pending_destroy.clear();
    }
};
```

### 4. Component Pool Pre-allocation

```cpp
template<typename T>
class ComponentPool : public IComponentPool
{
public:
    explicit ComponentPool(std::size_t initial_capacity)
        : entity_set(initial_capacity)
    {
        components.reserve(initial_capacity);
    }
};
```

### 5. Cache Query Views

```cpp
#include <typeindex>

class World
{
    // ... existing members ...
    std::unordered_map<std::type_index, std::any> view_cache;

public:
    template<typename... Components>
    auto query_cached()
    {
        auto key = std::type_index(typeid(std::tuple<Components...>));

        auto it = view_cache.find(key);
        if (it != view_cache.end())
            return std::any_cast<decltype(query<Components...>())>(it->second);

        auto view = query<Components...>();
        view_cache[key] = view;
        return view;
    }
};
```

---

## Summary

### Implementation Checklist

1. **Sparse Set** - Core data structure with O(1) operations
2. **Entity Manager** - Create, destroy, validate entities with generations
3. **Component Pool** - Store components using sparse sets
4. **World/Registry** - Central hub managing entities and component pools
5. **View System** - Efficient iteration over entities with specific components
6. **Systems** - Game logic that operates on component views

### Key Design Decisions

| Decision | Recommendation |
|----------|----------------|
| Entity ID size | `std::uint32_t` (4 billion entities) |
| Generation size | `std::uint32_t` (handle recycling safely) |
| Sparse array growth | Grow on demand, or pre-allocate to max |
| Component storage | One sparse set per component type |
| Multi-component queries | Iterate smallest pool, check others |
| Entity destruction | Deferred (batch at end of frame) |

### Performance Characteristics

| Operation | Complexity |
|-----------|------------|
| Create entity | O(1) |
| Destroy entity | O(C) where C = number of component types |
| Add component | O(1) |
| Remove component | O(1) |
| Get component | O(1) |
| Has component | O(1) |
| Iterate single component | O(N) where N = entities with component |
| Iterate multiple components | O(M * C) where M = smallest pool, C = component count |
