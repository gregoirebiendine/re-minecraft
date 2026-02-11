#ifndef FARFIELD_COLLISIONSYSTEM_H
#define FARFIELD_COLLISIONSYSTEM_H

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

            bool resolveAxis(Position& pos, Velocity& velocity, const CollisionBox& box, const int axis, const float prePos) const
            {
                constexpr float EPSILON = 0.001f;
                constexpr float CORNER_THRESHOLD = 0.01f;

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

                            // Check for full 3D AABB overlap
                            if (max.x <= blockMinVec.x || min.x >= blockMaxVec.x)
                                continue;
                            if (max.y <= blockMinVec.y || min.y >= blockMaxVec.y)
                                continue;
                            if (max.z <= blockMinVec.z || min.z >= blockMaxVec.z)
                                continue;

                            if (axis == 1)
                            {
                                // Y resolver: skip if entity was already overlapping on Y before movement
                                // (prevents walking beside a wall and getting pushed up)
                                const float preMin = prePos;
                                const float preMax = prePos + box.halfExtents.y * 2;
                                const float blockAxisMin = blockMinVec.y;
                                const float blockAxisMax = blockMaxVec.y;

                                if (preMax - blockAxisMin > EPSILON && blockAxisMax - preMin > EPSILON)
                                    continue;

                                // Y resolver: skip if cross-axis overlap is tiny
                                // (prevents catching on block corners while falling)
                                const float overlapX = std::min(max.x - blockMinVec.x, blockMaxVec.x - min.x);
                                const float overlapZ = std::min(max.z - blockMinVec.z, blockMaxVec.z - min.z);

                                if (overlapX < CORNER_THRESHOLD || overlapZ < CORNER_THRESHOLD)
                                    continue;
                            }

                            const float blockAxisMin = (axis == 0) ? blockMinVec.x : ((axis == 1) ? blockMinVec.y : blockMinVec.z);
                            const float blockAxisMax = (axis == 0) ? blockMaxVec.x : ((axis == 1) ? blockMaxVec.y : blockMaxVec.z);
                            const float playerMin = (axis == 0) ? min.x : ((axis == 1) ? min.y : min.z);
                            const float playerMax = (axis == 0) ? max.x : ((axis == 1) ? max.y : max.z);

                            const float penPositive = playerMax - blockAxisMin;
                            const float penNegative = blockAxisMax - playerMin;

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
                    const float preY = pos.y;
                    pos.y += vel.y;
                    const bool wasGoingDown = vel.y < 0.f;
                    const bool hitY = this->resolveAxis(pos, vel, box, 1, preY);

                    const float preX = pos.x;
                    pos.x += vel.x;
                    this->resolveAxis(pos, vel, box, 0, preX);

                    const float preZ = pos.z;
                    pos.z += vel.z;
                    this->resolveAxis(pos, vel, box, 2, preZ);

                    box.isGrounded = wasGoingDown && hitY;
                });
            }
    };
}

#endif