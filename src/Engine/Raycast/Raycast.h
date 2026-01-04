#ifndef RE_MINECRAFT_RAYCAST_H
#define RE_MINECRAFT_RAYCAST_H

#include <glm/vec3.hpp>

namespace Raycast
{
    struct Hit {
        bool hit = false;
        glm::ivec3 pos;
        glm::ivec3 previousPos;
    };

    constexpr float MAX_DISTANCE = 6.0f;
    constexpr float STEP = 0.05f;
}

#endif //RE_MINECRAFT_RAYCAST_H