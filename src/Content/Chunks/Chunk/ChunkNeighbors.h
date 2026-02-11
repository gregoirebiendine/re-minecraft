#ifndef FARFIELD_CHUNKNEIGHBORS_H
#define FARFIELD_CHUNKNEIGHBORS_H

#include "Chunk.h"

struct ChunkNeighbors {
    const Chunk* north;
    const Chunk* south;
    const Chunk* east;
    const Chunk* west;
    const Chunk* up;
    const Chunk* down;
};

#endif