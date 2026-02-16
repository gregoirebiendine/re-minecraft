#ifndef FARFIELD_MOVEMENTSYSTEM_H
#define FARFIELD_MOVEMENTSYSTEM_H

#include "ECS/ISystem.h"
#include "Components/Movements.h"
#include "Components/CollisionBox.h"

namespace ECS
{
    class MovementSystem : public ISystem
    {
        public:
            void update(Handler& handler, [[maybe_unused]] float dt) override
            {
                auto view = handler.query<Position, Velocity>();
                const auto& boxPool = handler.getPool<CollisionBox>();

                view.forEach([&](const EntityId id, Position& pos, const Velocity& vel)
                {
                    if (boxPool.has(id))
                        return;

                    pos += static_cast<const glm::vec3&>(vel);
                });
            }
    };
}

#endif
