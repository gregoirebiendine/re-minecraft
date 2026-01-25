#ifndef RE_MINECRAFT_DIRECTIONUTILS_H
#define RE_MINECRAFT_DIRECTIONUTILS_H

#include <iostream>
#include <glm/glm.hpp>
#include "Material.h"

namespace DirectionUtils
{
    inline std::string forwardVectorToCardinal(const glm::vec3& forwardVector)
    {
        const glm::vec3 f = glm::normalize(forwardVector);
        const float ax = abs(f.x);
        const float ay = abs(f.y);
        const float az = abs(f.z);

        if (ax > ay && ax > az)
            return (f.x > 0) ? "EAST" : "WEST";
        if (ay > az)
            return (f.y > 0) ? "UP" : "DOWN";
        return (f.z > 0) ? "SOUTH" : "NORTH";
    }

    inline MaterialFace getHorizontalFacing(const float yaw)
    {
        float normalizedYaw = yaw;

        if (normalizedYaw < 0)
            normalizedYaw += 360;

        if (normalizedYaw >= 315 || normalizedYaw < 45)
            return SOUTH;
        if (normalizedYaw >= 45 && normalizedYaw < 135)
            return WEST;
        if (normalizedYaw >= 135 && normalizedYaw < 225)
            return NORTH;
        return EAST;
    }

    inline MaterialFace getOppositeFacing(const MaterialFace facing)
    {
        switch (facing)
        {
            default:
            case NORTH:
                return SOUTH;
            case SOUTH:
                return NORTH;
            case WEST:
                return EAST;
            case EAST:
                return WEST;
        }
    }
}


#endif //RE_MINECRAFT_DIRECTIONUTILS_H