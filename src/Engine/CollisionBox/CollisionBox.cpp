#include "CollisionBox.h"

CollisionBox::CollisionBox(const glm::vec3 pos, const float w, const float h) :
    width(w),
    height(h)
{
    this->update(pos);
}

void CollisionBox::update(glm::vec3 pos)
{
    const float half = this->width / 2.0f;

    this->box.min = {pos.x - half, pos.y, pos.z - half};
    this->box.max = {pos.x + half, pos.y + height, pos.z + half};
}

bool CollisionBox::resolveAxis(glm::vec3& pos, glm::vec3& velocity, World& world, int axis)
{
    this->update(pos);

    const glm::ivec3 minBlock = glm::floor(this->box.min);
    const glm::ivec3 maxBlock = glm::floor(this->box.max);

    bool collided = false;

    for (int z = minBlock.z; z <= maxBlock.z; z++) {
        for (int y = minBlock.y; y <= maxBlock.y; y++) {
            for (int x = minBlock.x; x <= maxBlock.x; x++) {
                if (world.getBlock(x, y, z) == 0)
                    continue;

                const glm::vec3 blockMinVec = {static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)};
                const glm::vec3 blockMaxVec = {static_cast<float>(x + 1), static_cast<float>(y + 1), static_cast<float>(z + 1)};

                // Check for full 3D AABB overlap first
                if (this->box.max.x <= blockMinVec.x || this->box.min.x >= blockMaxVec.x)
                    continue;
                if (this->box.max.y <= blockMinVec.y || this->box.min.y >= blockMaxVec.y)
                    continue;
                if (this->box.max.z <= blockMinVec.z || this->box.min.z >= blockMaxVec.z)
                    continue;

                const float blockMin = (axis == 0) ? blockMinVec.x : ((axis == 1) ? blockMinVec.y : blockMinVec.z);
                const float blockMax = (axis == 0) ? blockMaxVec.x : ((axis == 1) ? blockMaxVec.y : blockMaxVec.z);
                const float playerMin = (axis == 0) ? this->box.min.x : ((axis == 1) ? this->box.min.y : this->box.min.z);
                const float playerMax = (axis == 0) ? this->box.max.x : ((axis == 1) ? this->box.max.y : this->box.max.z);

                const float penPositive = playerMax - blockMin;  // Penetration if moving +
                const float penNegative = blockMax - playerMin;  // Penetration if moving -

                // Choose push direction based on velocity (or smallest penetration if stationary)
                float velocityOnAxis = (axis == 0) ? velocity.x : ((axis == 1) ? velocity.y : velocity.z);
                float pushAmount;

                if (velocityOnAxis > 0.0001f) {
                    // Moving positive, push back negative
                    pushAmount = -penPositive;
                } else if (velocityOnAxis < -0.0001f) {
                    // Moving negative, push back positive
                    pushAmount = penNegative;
                } else {
                    // Stationary or very slow, use smallest penetration
                    pushAmount = (penPositive < penNegative) ? -penPositive : penNegative;
                }

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

                this->update(pos);
                collided = true;
            }
        }
    }

    return collided;
}

bool CollisionBox::resolveCollisions(glm::vec3& pos, glm::vec3& velocity, World& world)
{
    pos.y += velocity.y;
    const bool wasGoingDown = velocity.y < 0.f;
    const bool hitY = resolveAxis(pos, velocity, world, 1);

    pos.x += velocity.x;
    resolveAxis(pos, velocity, world, 0);

    pos.z += velocity.z;
    resolveAxis(pos, velocity, world, 2);

    return wasGoingDown && hitY;
}

BoundingBox CollisionBox::getBoundingBox() const
{
    return this->box;
}
