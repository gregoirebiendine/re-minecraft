#ifndef RE_MINECRAFT_COLLISIONBOX_H
#define RE_MINECRAFT_COLLISIONBOX_H

#include <iostream>
#include <glm/glm.hpp>
#include "World.h"

struct BoundingBox
{
    glm::vec3 min{};
    glm::vec3 max{};
};

class CollisionBox
{
    float width, height;
    BoundingBox box;

    bool resolveAxis(glm::vec3& pos, glm::vec3& velocity, World& world, int axis);

    public:
        CollisionBox(glm::vec3 pos, float w, float h);

        void update(glm::vec3 pos);
        bool resolveCollisions(glm::vec3& pos, glm::vec3& velocity, World& world);
        BoundingBox getBoundingBox() const;
};

#endif //RE_MINECRAFT_COLLISIONBOX_H