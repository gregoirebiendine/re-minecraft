#ifndef FARFIELD_DIRECTIONUTILS_H
#define FARFIELD_DIRECTIONUTILS_H

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

    inline MaterialFace getHorizontalFacing(const glm::vec3 forwardVector)
    {
        const auto horizontal = glm::normalize(glm::vec3{forwardVector.x, 0.f, forwardVector.z});

        if (abs(horizontal.x) > abs(horizontal.z))
            return horizontal.x > 0 ? EAST : WEST;
        return horizontal.z > 0 ? SOUTH : NORTH;
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

    inline BlockRotation getAxisFromHitFace(const MaterialFace hitFace)
    {
        // Returns 4-6 to skip UV rotation in shader (rotation < 4 check)
        switch (hitFace)
        {
            default:
            case UP:
            case DOWN:
                return 4;  // Y-axis (vertical)
            case NORTH:
            case SOUTH:
                return 5;  // Z-axis (horizontal, pointing N/S)
            case EAST:
            case WEST:
                return 6;  // X-axis (horizontal, pointing E/W)
        }
    }
}


#endif