#ifndef RE_MINECRAFT_COLLISIONBOX_H
#define RE_MINECRAFT_COLLISIONBOX_H

#include "glm/vec3.hpp"

namespace ECS
{
    struct CollisionBox
    {
        glm::vec3 halfExtents;
        bool isGrounded = false;
    };
}

#endif //RE_MINECRAFT_COLLISIONBOX_H