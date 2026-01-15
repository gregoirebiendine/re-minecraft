#ifndef RE_MINECRAFT_UTILS_H
#define RE_MINECRAFT_UTILS_H

#include "ChunkPos.h"
#include "Chunk.h"

namespace ChunkCoords
{
    // Convert a 1D index to 3D coordinates (x, y, z)
    inline BlockPos indexToLocalCoords(const int index)
    {
        return {
            index / (Chunk::SIZE * Chunk::SIZE),
            (index / Chunk::SIZE) % Chunk::SIZE,
            index % Chunk::SIZE
        };
    }

    // Convert a local 3D coordinates to a 1D index
    inline int localCoordsToIndex(const uint8_t lx, const uint8_t ly, const uint8_t lz)
    {
        return lx + Chunk::SIZE * (ly + Chunk::SIZE * lz);
    }
}

#endif //RE_MINECRAFT_UTILS_H