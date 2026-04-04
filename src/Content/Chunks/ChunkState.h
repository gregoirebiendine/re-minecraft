#ifndef FARFIELD_CHUNKSTATE_H
#define FARFIELD_CHUNKSTATE_H

enum class ChunkState
{
    UNLOADED,

    // Terrain
    TERRAIN_PENDING,
    TERRAIN_GENERATING,
    TERRAIN_DONE,

    // Meshing
    MESHING,
    MESHED,
    READY
};

inline bool hasTerrainComplete(const ChunkState state) {
    return state >= ChunkState::TERRAIN_DONE;
}

#endif