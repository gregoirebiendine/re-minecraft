#ifndef RE_MINECRAFT_CAMERA_COMP_H
#define RE_MINECRAFT_CAMERA_COMP_H

#include "glm/vec3.hpp"

namespace ECS
{
    struct Camera
    {
        float fov = 90.f;
        float sensitivity = 0.1f;
        float nearPlane = 0.1f;
        float farPlane = 512.f;
        float yaw = 0.f;
        float pitch = 0.f;
        glm::vec3 eyeOffset = {0, 1.7f, 0};
    };
}

#endif