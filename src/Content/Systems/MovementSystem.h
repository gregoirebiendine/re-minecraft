#ifndef RE_MINECRAFT_MOVEMENTSYSTEM_H
#define RE_MINECRAFT_MOVEMENTSYSTEM_H

#include "ECS/ISystem.h"
#include "Components/MovementComponent.h"

namespace ECS
{
    class MovementSystem : public IISystem
    {
        public:
            void update(Handler& handler, float dt) override
            {
                auto view = handler.query<Position, Velocity>();

                view.forEach([dt]([[maybe_unused]] EntityId id, Position& pos, const Velocity& vel)
                {
                    pos += vel * dt;
                });
            }
    };
}

#endif