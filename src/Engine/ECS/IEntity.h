#ifndef FARFIELD_IENTITY_H
#define FARFIELD_IENTITY_H

#include <cstdint>
#include <vector>
#include <algorithm>
#include <memory>
#include <limits>
#include <ranges>

namespace ECS
{
    using EntityId = std::uint32_t;
    using Index = std::uint32_t;

    inline static constexpr Index INVALID_INDEX = std::numeric_limits<Index>::max();
    inline static constexpr EntityId INVALID_ENTITY_ID = std::numeric_limits<EntityId>::max();

    struct IEntity
    {
        std::uint32_t id = INVALID_ENTITY_ID;
        std::uint32_t generation = 0;

        static IEntity makeEntity(const std::uint32_t id, const std::uint32_t generation)
        {
            return IEntity{id, generation};
        }
    };

    class SparseSet
    {
        std::vector<Index> sparse;
        std::vector<EntityId> dense;
        std::size_t count = 0;

        public:
            explicit SparseSet(const std::size_t max_entities = 0) :
                sparse(max_entities, INVALID_INDEX),
                dense(max_entities)
            {}

            [[nodiscard]] bool contains(const EntityId id) const
            {
                if (id >= this->sparse.size())
                    return false;

                const Index denseIndex = this->sparse[id];

                if (denseIndex == INVALID_INDEX || denseIndex >= count)
                    return false;

                return this->dense[denseIndex] == id;
            }

            Index insert(const EntityId id)
            {
                if (id >= sparse.size())
                    sparse.resize(id + 1, INVALID_INDEX);

                if (contains(id))
                    return sparse[id];

                const auto newIndex = static_cast<Index>(count);

                if (newIndex >= dense.size())
                    dense.resize(newIndex + 1);

                dense[newIndex] = id;
                sparse[id] = newIndex;
                ++count;

                return newIndex;
            }

            bool remove(const EntityId id)
            {
                if (!this->contains(id))
                    return false;

                const Index removed_index = sparse[id];
                const auto last_index = static_cast<Index>(count - 1);
                const EntityId last_entity = dense[last_index];

                // Swap last element into the removed slot
                dense[removed_index] = last_entity;
                sparse[last_entity] = removed_index;

                // Mark removed entity as invalid
                sparse[id] = INVALID_INDEX;
                --count;

                return true;
            }

            [[nodiscard]] Index getIndex(const EntityId id) const
            {
                if (!this->contains(id))
                    return INVALID_INDEX;
                return sparse[id];
            }

            [[nodiscard]] std::size_t size() const { return count; }

            [[nodiscard]] const std::vector<EntityId>& getDense() const { return dense; }
    };

    class EntityManager
    {
        std::vector<std::uint32_t> generations;
        std::vector<std::uint32_t> freeList;
        std::uint32_t nextId = 0;

        public:
            EntityManager() = default;

            IEntity create()
            {
                if (!freeList.empty())
                {
                    std::uint32_t id = freeList.back();
                    freeList.pop_back();
                    return IEntity::makeEntity(id, generations[id]);
                }

                const std::uint32_t id = nextId++;
                generations.push_back(0);
                return IEntity::makeEntity(id, 0);
            }

            bool destroy(const IEntity entity)
            {
                if (!this->isAlive(entity))
                    return false;

                ++generations[entity.id];
                freeList.push_back(entity.id);

                return true;
            }

            [[nodiscard]] bool isAlive(const IEntity entity) const
            {
                if (entity.id >= generations.size())
                    return false;
                return generations[entity.id] == entity.generation;
            }
    };
}

#endif