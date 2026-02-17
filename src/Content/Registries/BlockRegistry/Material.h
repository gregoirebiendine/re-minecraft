#ifndef FARFIELD_MATERIAL_H
#define FARFIELD_MATERIAL_H

#include <cstdint>

constexpr int ROTATION_SHIFT = 13;
constexpr int BLOCK_ID_MASK = 0x1FFF;
constexpr int ROTATION_MASK = 0x7;

using BlockId = std::uint16_t;       // 16bits
using BlockRotation = std::uint8_t;  // 8bits

enum MaterialFace : unsigned char
{
    NORTH,
    SOUTH,
    WEST,
    EAST,
    UP,
    DOWN
};

struct Material {
    std::uint16_t data = 0;

    static Material pack(const BlockId blockId, const BlockRotation rotation)
    {
        return {static_cast<std::uint16_t>((rotation << ROTATION_SHIFT) | (blockId & BLOCK_ID_MASK)) };
    }

    [[nodiscard]] BlockId getBlockId() const { return data & BLOCK_ID_MASK; }
    [[nodiscard]] BlockRotation getRotation() const { return (data >> ROTATION_SHIFT) & ROTATION_MASK; }

    bool operator==(const Material&) const = default;
};

static_assert(sizeof(Material) == 2);

#endif