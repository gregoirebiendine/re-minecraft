#ifndef FARFIELD_PLAYERINPUT_H
#define FARFIELD_PLAYERINPUT_H

#include <cstdint>
#include <glm/glm.hpp>
#include "Material.h"

namespace ECS
{
    struct PlayerInput
    {
        glm::vec3 direction{0.f};
        std::uint8_t jumpBufferFrames = 0;
    };
}

#endif