#ifndef RE_MINECRAFT_MATERIAL_H
#define RE_MINECRAFT_MATERIAL_H
#include "BlockRegistry.h"

constexpr int ROTATION_SHIFT = 13;
constexpr int BLOCK_ID_MASK = 0x1FFF;
constexpr int ROTATION_MASK = 0x7;

using Material = uint16_t;
using BlockId = uint16_t;
using BlockRotation = uint8_t;

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

}


#endif //RE_MINECRAFT_MATERIAL_H