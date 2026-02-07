#ifndef RE_MINECRAFT_CHUNKNEIGHBORS_H
#define RE_MINECRAFT_CHUNKNEIGHBORS_H

#include "Chunk.h"

struct ChunkNeighbors {
    const Chunk* north;
    const Chunk* south;
    const Chunk* east;
    const Chunk* west;
    const Chunk* up;
    const Chunk* down;
};

#endif //RE_MINECRAFT_CHUNKNEIGHBORS_H