#ifndef FARFIELD_RAYCAST_H
#define FARFIELD_RAYCAST_H

#include <glm/vec3.hpp>

#include "Material.h"
#include "World.h"

namespace Raycast
{
    constexpr float MAX_DISTANCE = 6.0f;
    constexpr float STEP = 0.05f;

    struct Hit {
        bool hit = false;
        MaterialFace hitFace = UP;
        glm::ivec3 pos{};
        glm::ivec3 previousPos{};
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

    inline Hit cast(World& world, const glm::vec3 origin, const glm::vec3 dir, const float dist = MAX_DISTANCE)
    {
        glm::ivec3 lastBlock(-1);
        float t = 0.f;

        while (t < dist) {
            glm::ivec3 blockPos = glm::floor(origin + dir * t);

            if (blockPos == lastBlock) {
                t += STEP;
                continue;
            }

            lastBlock = blockPos;

            if (world.getBlock(blockPos.x, blockPos.y, blockPos.z)) {
                const glm::ivec3 previousPos = glm::floor(origin + dir * (t - STEP));
                const glm::ivec3 diff = blockPos - previousPos;
                const MaterialFace hitFace = Raycast::calculateHitFace(diff);

                return {
                    true,
                    hitFace,
                    blockPos,
                    previousPos
                };
            }

            t += STEP;
        }

        return {};
    }
}

#endif