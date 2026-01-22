#ifndef RE_MINECRAFT_RAYCAST_H
#define RE_MINECRAFT_RAYCAST_H

#include <glm/vec3.hpp>

namespace Raycast
{
    struct Hit {
        bool hit = false;
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

    constexpr float MAX_DISTANCE = 6.0f;
    constexpr float STEP = 0.05f;
}

#endif //RE_MINECRAFT_RAYCAST_H