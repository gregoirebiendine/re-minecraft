#ifndef RE_MINECRAFT_COLLISIONSYSTEM_H
#define RE_MINECRAFT_COLLISIONSYSTEM_H

#pragma once

class World;

#include "World.h"
#include "ECS/ISystem.h"
#include "Components/Movements.h"
#include "Components/CollisionBox.h"

namespace ECS
{
    class CollisionSystem : public ISystem
    {
        World& world;

        public:
            explicit CollisionSystem(World& _world) : world(_world) {};

            bool resolveAxis(Position& pos, Velocity& velocity, const CollisionBox& box, const int axis) const
            {
                glm::vec3 min = {pos.x - box.halfExtents.x, pos.y, pos.z - box.halfExtents.z};
                glm::vec3 max = {pos.x + box.halfExtents.x, pos.y + box.halfExtents.y * 2, pos.z + box.halfExtents.z};
                const glm::ivec3 minBlock = glm::floor(min);
                const glm::ivec3 maxBlock = glm::floor(max);

                bool collided = false;

                for (int z = minBlock.z; z <= maxBlock.z; z++) {
                    for (int y = minBlock.y; y <= maxBlock.y; y++) {
                        for (int x = minBlock.x; x <= maxBlock.x; x++) {
                            if (world.isAir(x, y, z))
                                continue;

                            const glm::vec3 blockMinVec = {static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)};
                            const glm::vec3 blockMaxVec = {static_cast<float>(x + 1), static_cast<float>(y + 1), static_cast<float>(z + 1)};

                            // Check for full 3D AABB overlap first
                            if (max.x <= blockMinVec.x || min.x >= blockMaxVec.x)
                                continue;
                            if (max.y <= blockMinVec.y || min.y >= blockMaxVec.y)
                                continue;
                            if (max.z <= blockMinVec.z || min.z >= blockMaxVec.z)
                                continue;

                            const float blockMin = (axis == 0) ? blockMinVec.x : ((axis == 1) ? blockMinVec.y : blockMinVec.z);
                            const float blockMax = (axis == 0) ? blockMaxVec.x : ((axis == 1) ? blockMaxVec.y : blockMaxVec.z);
                            const float playerMin = (axis == 0) ? min.x : ((axis == 1) ? min.y : min.z);
                            const float playerMax = (axis == 0) ? max.x : ((axis == 1) ? max.y : max.z);

                            const float penPositive = playerMax - blockMin;
                            const float penNegative = blockMax - playerMin;

                            const float velocityOnAxis = (axis == 0) ? velocity.x : ((axis == 1) ? velocity.y : velocity.z);
                            float pushAmount;

                            if (velocityOnAxis > 0.0001f)
                                pushAmount = -penPositive;
                            else if (velocityOnAxis < -0.0001f)
                                pushAmount = penNegative;
                            else
                                pushAmount = (penPositive < penNegative) ? -penPositive : penNegative;

                            // Apply correction
                            if (axis == 0) {
                                pos.x += pushAmount;
                                velocity.x = 0.f;
                            } else if (axis == 1) {
                                pos.y += pushAmount;
                                velocity.y = 0.f;
                            } else {
                                pos.z += pushAmount;
                                velocity.z = 0.f;
                            }

                            min = {pos.x - box.halfExtents.x, pos.y, pos.z - box.halfExtents.z};
                            max = {pos.x + box.halfExtents.x, pos.y + box.halfExtents.y * 2, pos.z + box.halfExtents.z};

                            collided = true;
                        }
                    }
                }

                return collided;
            }

            void update(Handler& handler, [[maybe_unused]] float dt) override
            {
                auto view = handler.query<Position, Velocity, CollisionBox>();

                view.forEach([&]([[maybe_unused]] EntityId id, Position& pos, Velocity& vel, CollisionBox& box)
                {
                    pos.y += vel.y;
                    const bool wasGoingDown = vel.y < 0.f;
                    const bool hitY = this->resolveAxis(pos, vel, box, 1);

                    pos.x += vel.x;
                    this->resolveAxis(pos, vel, box, 0);

                    pos.z += vel.z;
                    this->resolveAxis(pos, vel, box, 2);

                    box.isGrounded = wasGoingDown && hitY;
                });
            }
    };
}

#endif