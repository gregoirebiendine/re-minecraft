#ifndef RE_MINECRAFT_MATERIAL_H
#define RE_MINECRAFT_MATERIAL_H

#include <cstdint>

constexpr int ROTATION_SHIFT = 13;
constexpr int BLOCK_ID_MASK = 0x1FFF;
constexpr int ROTATION_MASK = 0x7;

using Material = std::uint16_t;      // 16bits
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

namespace BlockData
{
    inline Material packBlockData(const BlockId blockId, const BlockRotation rotation)
    {
        return (rotation << ROTATION_SHIFT) | (blockId & BLOCK_ID_MASK);
    }

    inline BlockId getBlockId(const Material mat)
    {
        return mat & BLOCK_ID_MASK;
    }

    inline BlockRotation getRotation(const Material mat)
    {
        return (mat >> ROTATION_SHIFT) & ROTATION_MASK;
    }

    inline MaterialFace remapFaceForRotation(const MaterialFace face, const BlockRotation rotation)
    {
        if (face == UP or face == DOWN)
            return face;

        constexpr MaterialFace FACE_REMAP[4][4] = {
            {NORTH, SOUTH, WEST, EAST},
            {SOUTH, NORTH, EAST, WEST},
            {EAST, WEST, NORTH, SOUTH},
            {WEST, EAST, SOUTH, NORTH}
        };

        return FACE_REMAP[rotation][face];
    }

    inline MaterialFace remapFaceForAxisRotation(const MaterialFace face, const BlockRotation rotation)
    {
        // Rotation 4 = Y-axis (vertical, no remapping needed)
        if (rotation == 4)
            return face;

        // Rotation 5 = Z-axis (log pointing N/S)
        if (rotation == 5)
            switch (face)
            {
                case UP:    return SOUTH;
                case DOWN:  return NORTH;
                case NORTH: return DOWN;
                case SOUTH: return UP;
                default:    return face;
            }

        // Rotation 6 = X-axis (log pointing E/W)
        if (rotation == 6)
            switch (face)
            {
                case UP:    return EAST;
                case DOWN:  return WEST;
                case EAST:  return DOWN;
                case WEST:  return UP;
                default:    return face;
            }

        return face;
    }

}

#endif //RE_MINECRAFT_MATERIAL_H