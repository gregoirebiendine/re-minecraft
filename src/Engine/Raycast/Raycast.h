#ifndef FARFIELD_RAYCAST_H
#define FARFIELD_RAYCAST_H

#include <glm/vec3.hpp>

#include "Material.h"
#include "World.h"
#include "Components/Movements.h"
#include "Components/CollisionBox.h"

namespace Raycast
{
    constexpr float MAX_DISTANCE = 6.0f;
    constexpr float STEP = 0.05f;

    enum class HitType : unsigned char
    {
        BLOCK,
        ENTITY,
        NONE
    };

    struct Hit {
        bool hit = false;
        HitType hitType = HitType::NONE;
        MaterialFace hitFace = UP;
        glm::ivec3 pos{};
        glm::ivec3 previousPos{};

        [[nodiscard]] bool hasHitBlock() const
        {
            return hit && hitType == HitType::BLOCK;
        }

        [[nodiscard]] bool hasHitEntity() const
        {
            return hit && hitType == HitType::ENTITY;
        }
    };

    inline MaterialFace calculateHitFace(const glm::ivec3& diff)
    {
        if (diff.x == 1)  return WEST;
        if (diff.x == -1) return EAST;
        if (diff.y == 1)  return DOWN;
        if (diff.y == -1) return UP;
        if (diff.z == 1)  return NORTH;
        if (diff.z == -1) return SOUTH;

        return UP;
    }

    inline bool intersectAABB(const glm::vec3& origin, const glm::vec3& dir, const glm::vec3& aabbMin, const glm::vec3& aabbMax, const float maxDist, float& tHit)
    {
        const glm::vec3 dirFrac = 1.0f / dir;

        const float t1 = (aabbMin.x - origin.x) * dirFrac.x;
        const float t2 = (aabbMax.x - origin.x) * dirFrac.x;
        const float t3 = (aabbMin.y - origin.y) * dirFrac.y;
        const float t4 = (aabbMax.y - origin.y) * dirFrac.y;
        const float t5 = (aabbMin.z - origin.z) * dirFrac.z;
        const float t6 = (aabbMax.z - origin.z) * dirFrac.z;

        float tmin = glm::max(glm::max(glm::min(t1, t2), glm::min(t3, t4)), glm::min(t5, t6));
        float tmax = glm::min(glm::min(glm::max(t1, t2), glm::max(t3, t4)), glm::max(t5, t6));

        if (tmax < 0.0f) return false;     // AABB is behind the ray
        if (tmin > tmax) return false;      // ray misses the AABB
        if (tmin > maxDist) return false;   // AABB is too far away

        tHit = tmin;
        return true;
    }

    inline Hit cast(World& world, const glm::vec3 origin, const glm::vec3 dir, const float dist = MAX_DISTANCE)
    {
        Hit blockHit{};
        float blockDist = dist;
        float t = STEP;

        // Iterate along raycast for blocks
        while (t < dist) {
            glm::vec3 pos = origin + dir * t;

            if (world.getBlock(glm::floor(pos))) {
                const glm::ivec3 previousPos = glm::floor(origin + dir * (t - STEP));
                const glm::ivec3 diff = glm::ivec3(pos) - previousPos;
                const MaterialFace hitFace = calculateHitFace(diff);

                blockHit = {
                    true,
                    HitType::BLOCK,
                    hitFace,
                    pos,
                    previousPos
                };
                break;
            }

            t += STEP;
        }


        Hit entityHit{};
        float closestEntityDist = blockDist;
        auto view = world.getECS().query<ECS::Position, ECS::CollisionBox>();

        // Iterate over entities to check along raycast
        view.forEach([&](const ECS::EntityId id, const ECS::Position& pos, const ECS::CollisionBox& box)
        {
            if (id == world.getPlayerEntity().id)
                return;

            const glm::vec3 aabbMin = {pos.x - box.halfExtents.x, pos.y, pos.z - box.halfExtents.z};
            const glm::vec3 aabbMax = {pos.x + box.halfExtents.x, pos.y + box.halfExtents.y * 2, pos.z + box.halfExtents.z};

            float tHit;
            if (intersectAABB(origin, dir, aabbMin, aabbMax, closestEntityDist, tHit)) {
                closestEntityDist = tHit;
                entityHit = {
                    true,
                    HitType::ENTITY,
                    UP,
                    glm::ivec3(origin + dir * tHit),
                    glm::ivec3(0)
                };
            }
        });

        if (entityHit.hit) return entityHit;
        if (blockHit.hit) return blockHit;
        return {};
    }
}

#endif