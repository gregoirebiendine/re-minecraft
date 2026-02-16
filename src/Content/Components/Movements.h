#ifndef FARFIELD_MOVEMENTCOMPONENT_H
#define FARFIELD_MOVEMENTCOMPONENT_H

#include <ostream>
#include <glm/glm.hpp>

namespace ECS
{
    struct Position : glm::vec3
    {
        using glm::vec3::vec3;

        friend std::ostream& operator<<(std::ostream& os, const Position& pos)
        {
            return os << "P(" << pos.x << ", " << pos.y << ", " << pos.z << ")";
        }
    };

    struct Velocity : glm::vec3
    {
        using glm::vec3::vec3;

        friend std::ostream& operator<<(std::ostream& os, const Velocity& vel)
        {
            return os << "V(" << vel.x << ", " << vel.y << ", " << vel.z << ")";
        }
    };

    struct Rotation : glm::vec3
    {
        using glm::vec3::vec3;

        friend std::ostream& operator<<(std::ostream& os, const Rotation& rot)
        {
            return os << "R(" << rot.x << ", " << rot.y << ", " << rot.z << ")";
        }
    };
}

#endif