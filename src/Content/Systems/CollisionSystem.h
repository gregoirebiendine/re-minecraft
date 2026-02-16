#ifndef FARFIELD_COLLISIONSYSTEM_H
#define FARFIELD_COLLISIONSYSTEM_H

#pragma once

class World;

#include "World.h"
#include "ECS/ISystem.h"
#include "Components/Movements.h"
#include "Components/CollisionBox.h"

#include <cmath>
#include <glm/glm.hpp>

namespace ECS
{
    inline glm::vec3 computeRotatedHalfExtents(const glm::vec3& halfExtents, float yawDegrees)
    {
        const float rad = glm::radians(yawDegrees);
        const float cosA = std::abs(std::cos(rad));
        const float sinA = std::abs(std::sin(rad));

        return {
            halfExtents.x * cosA + halfExtents.z * sinA,
            halfExtents.y,
            halfExtents.x * sinA + halfExtents.z * cosA
        };
    }

    struct AABB
    {
        glm::vec3 min;
        glm::vec3 max;
    };

    inline AABB computeAABB(const Position& pos, const glm::vec3& rotatedHalf)
    {
        return {
            { pos.x - rotatedHalf.x, pos.y, pos.z - rotatedHalf.z },
            { pos.x + rotatedHalf.x, pos.y + rotatedHalf.y * 2.0f, pos.z + rotatedHalf.z }
        };
    }

    class CollisionSystem : public ISystem
    {
        World& world;

        public:
            explicit CollisionSystem(World& _world) : world(_world) {};

            bool resolveAxis(Position& pos, Velocity& vel, const glm::vec3& halfExt, const int axis, const float preAxisPos = std::numeric_limits<float>::quiet_NaN()) const
            {
                AABB entity = computeAABB(pos, halfExt);

                const glm::ivec3 minBlock = glm::ivec3(glm::floor(entity.min));
                const glm::ivec3 maxBlock = glm::ivec3(glm::floor(entity.max));

                const bool checkPreOverlap = !std::isnan(preAxisPos);
                float preMin = 0.0f, preMax = 0.0f;
                if (checkPreOverlap)
                {
                    if (axis == 1) { preMin = preAxisPos; preMax = preAxisPos + halfExt.y * 2.0f; }
                    else if (axis == 0) { preMin = preAxisPos - halfExt.x; preMax = preAxisPos + halfExt.x; }
                    else { preMin = preAxisPos - halfExt.z; preMax = preAxisPos + halfExt.z; }
                }

                bool collided = false;

                for (int bz = minBlock.z; bz <= maxBlock.z; ++bz)
                {
                    for (int by = minBlock.y; by <= maxBlock.y; ++by)
                    {
                        for (int bx = minBlock.x; bx <= maxBlock.x; ++bx)
                        {
                            if (this->world.isAir(bx, by, bz))
                                continue;

                            const glm::vec3 blockMin = { static_cast<float>(bx), static_cast<float>(by), static_cast<float>(bz) };
                            const glm::vec3 blockMax = { static_cast<float>(bx + 1), static_cast<float>(by + 1), static_cast<float>(bz + 1) };

                            if (entity.max.x <= blockMin.x || entity.min.x >= blockMax.x)
                                continue;
                            if (entity.max.y <= blockMin.y || entity.min.y >= blockMax.y)
                                continue;
                            if (entity.max.z <= blockMin.z || entity.min.z >= blockMax.z)
                                continue;

                            // Skip blocks already overlapping on this axis before the move
                            if (checkPreOverlap)
                            {
                                const float bMin = (axis == 0) ? blockMin.x : (axis == 1) ? blockMin.y : blockMin.z;
                                const float bMax = (axis == 0) ? blockMax.x : (axis == 1) ? blockMax.y : blockMax.z;

                                if (preMax > bMin && bMax > preMin)
                                    continue;
                            }

                            const float penPos = (axis == 0) ? entity.max.x - blockMin.x
                                               : (axis == 1) ? entity.max.y - blockMin.y
                                                             : entity.max.z - blockMin.z;
                            const float penNeg = (axis == 0) ? blockMax.x - entity.min.x
                                               : (axis == 1) ? blockMax.y - entity.min.y
                                                             : blockMax.z - entity.min.z;

                            const float velOnAxis = (axis == 0) ? vel.x : (axis == 1) ? vel.y : vel.z;
                            float push;

                            if (velOnAxis > 0.0001f)
                                push = -penPos;
                            else if (velOnAxis < -0.0001f)
                                push = penNeg;
                            else
                                push = (penPos < penNeg) ? -penPos : penNeg;

                            if (axis == 0) pos.x += push;
                            else if (axis == 1) pos.y += push;
                            else pos.z += push;

                            if (axis == 0) vel.x = 0.0f;
                            else if (axis == 1) vel.y = 0.0f;
                            else vel.z = 0.0f;

                            entity = computeAABB(pos, halfExt);
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
                    const glm::vec3& halfExt = box.halfExtents;

                    const bool wasGoingDown = vel.y < 0.0f;
                    const float preY = pos.y;

                    pos.y += vel.y;
                    const bool hitY = resolveAxis(pos, vel, halfExt, 1, preY);

                    pos.x += vel.x;
                    resolveAxis(pos, vel, halfExt, 0);

                    pos.z += vel.z;
                    resolveAxis(pos, vel, halfExt, 2);

                    box.isGrounded = wasGoingDown && hitY;
                });
            }
    };
}

#endif
