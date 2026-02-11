#ifndef FARFIELD_COLLISIONBOX_H
#define FARFIELD_COLLISIONBOX_H

#include "glm/vec3.hpp"

namespace ECS
{
    struct CollisionBox
    {
        glm::vec3 halfExtents;
        bool isGrounded = false;
    };
}

#endif