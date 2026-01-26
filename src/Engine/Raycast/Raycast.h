#ifndef RE_MINECRAFT_RAYCAST_H
#define RE_MINECRAFT_RAYCAST_H

#include <glm/vec3.hpp>
#include "Material.h"

namespace Raycast
{
    constexpr float MAX_DISTANCE = 6.0f;
    constexpr float STEP = 0.05f;

    struct Hit {
        bool hit = false;
        MaterialFace hitFace;
        glm::ivec3 pos;
        glm::ivec3 previousPos;

        bool operator==(const Hit& other) const
        {
            if (this->hit != other.hit)
                return false;
            if (!this->hit)
                return true;
            return this->pos == other.pos && this->previousPos == other.previousPos;
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
}

#endif //RE_MINECRAFT_RAYCAST_H