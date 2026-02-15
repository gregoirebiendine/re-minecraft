#ifndef FARFIELD_CROSSHAIRVERTICES_H
#define FARFIELD_CROSSHAIRVERTICES_H

#include <vector>
#include <glm/glm.hpp>

inline constexpr float CH_SIZE = 20.f;
inline constexpr float CH_THICKNESS = 4.f;
inline constexpr float CH_OFFSET = 3.f;

inline std::vector<GuiVertex> getCrosshairVertices(const glm::ivec2& vpSize)
{
    const glm::vec2 mid = {vpSize.x / 2 , vpSize.y / 2};
    constexpr glm::vec4 c{0.78431f,0.78431f,0.78431f,0.8f};

    return {
        // Horizontal left
        {{ mid.x - (CH_SIZE / 2) - CH_OFFSET, mid.y - (CH_THICKNESS / 2) }, {-1.f, -1.f}, c},
        {{ mid.x - (CH_SIZE / 2) - CH_OFFSET, mid.y - (CH_THICKNESS / 2) + CH_THICKNESS}, {-1.f, -1.f}, c},
        {{ mid.x - (CH_SIZE / 2) - CH_OFFSET + (CH_SIZE / 2), mid.y - (CH_THICKNESS / 2) + CH_THICKNESS}, {-1.f, -1.f}, c},
        {{ mid.x - (CH_SIZE / 2) - CH_OFFSET, mid.y - (CH_THICKNESS / 2)}, {-1.f, -1.f}, c},
        {{ mid.x - (CH_SIZE / 2) - CH_OFFSET + (CH_SIZE / 2), mid.y - (CH_THICKNESS / 2) + CH_THICKNESS}, {-1.f, -1.f}, c},
        {{ mid.x - (CH_SIZE / 2) - CH_OFFSET + (CH_SIZE / 2), mid.y - (CH_THICKNESS / 2)}, {-1.f, -1.f}, c},

        // Horizontal right
        {{ mid.x + CH_OFFSET, mid.y - (CH_THICKNESS / 2)}, {-1.f, -1.f}, c},
        {{ mid.x + CH_OFFSET, mid.y - (CH_THICKNESS / 2) + CH_THICKNESS}, {-1.f, -1.f}, c},
        {{ mid.x + CH_OFFSET + (CH_SIZE / 2), mid.y - (CH_THICKNESS / 2) + CH_THICKNESS}, {-1.f, -1.f}, c},
        {{ mid.x + CH_OFFSET, mid.y - (CH_THICKNESS / 2)}, {-1.f, -1.f}, c},
        {{ mid.x + CH_OFFSET + (CH_SIZE / 2), mid.y - (CH_THICKNESS / 2) + CH_THICKNESS}, {-1.f, -1.f}, c},
        {{ mid.x + CH_OFFSET + (CH_SIZE / 2), mid.y - (CH_THICKNESS / 2)}, {-1.f, -1.f}, c},

        // Vertical top
        {{ mid.x - (CH_THICKNESS / 2), mid.y - (CH_SIZE / 2) - CH_OFFSET }, {-1.f, -1.f}, c},
        {{ mid.x - (CH_THICKNESS / 2), mid.y - (CH_SIZE / 2) - CH_OFFSET + (CH_SIZE / 2) }, {-1.f, -1.f}, c},
        {{ mid.x - (CH_THICKNESS / 2) + CH_THICKNESS, mid.y - (CH_SIZE / 2) - CH_OFFSET + (CH_SIZE / 2) }, {-1.f, -1.f}, c},
        {{ mid.x - (CH_THICKNESS / 2), mid.y - (CH_SIZE / 2) - CH_OFFSET }, {-1.f, -1.f}, c},
        {{ mid.x - (CH_THICKNESS / 2) + CH_THICKNESS, mid.y - (CH_SIZE / 2) - CH_OFFSET + (CH_SIZE / 2) }, {-1.f, -1.f}, c},
        {{ mid.x - (CH_THICKNESS / 2) + CH_THICKNESS, mid.y - (CH_SIZE / 2) - CH_OFFSET }, {-1.f, -1.f}, c},

        // Vertical bottom
        {{ mid.x - (CH_THICKNESS / 2), mid.y + CH_OFFSET }, {-1.f, -1.f}, c},
        {{ mid.x - (CH_THICKNESS / 2), mid.y + CH_OFFSET + (CH_SIZE / 2) }, {-1.f, -1.f}, c},
        {{ mid.x - (CH_THICKNESS / 2) + CH_THICKNESS, mid.y + CH_OFFSET + (CH_SIZE / 2) }, {-1.f, -1.f}, c},
        {{ mid.x - (CH_THICKNESS / 2), mid.y + CH_OFFSET }, {-1.f, -1.f}, c},
        {{ mid.x - (CH_THICKNESS / 2) + CH_THICKNESS, mid.y + CH_OFFSET + (CH_SIZE / 2) }, {-1.f, -1.f}, c},
        {{ mid.x - (CH_THICKNESS / 2) + CH_THICKNESS, mid.y + CH_OFFSET }, {-1.f, -1.f}, c},
    };
}

#endif
