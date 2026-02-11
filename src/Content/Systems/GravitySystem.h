#ifndef RE_MINECRAFT_GRAVITYSYSTEM_H
#define RE_MINECRAFT_GRAVITYSYSTEM_H

#include "ECS/ISystem.h"
#include "Components/Movements.h"
#include "Components/Gravity.h"
#include "Components/CollisionBox.h"

namespace ECS
{
    class GravitySystem : public ISystem
    {
        public:
            void update(Handler& handler, [[maybe_unused]] float dt) override
            {
                auto view = handler.query<Velocity, Gravity, CollisionBox>();

                view.forEach([&]([[maybe_unused]] EntityId id, Velocity& vel, const Gravity& gravity, const CollisionBox& box)
                {
                    if (box.isGrounded)
                        return;
                    vel.y -= gravity.strength;
                    vel.y = glm::max(vel.y, gravity.terminalVelocity);
                });
            }
    };
}

#endif