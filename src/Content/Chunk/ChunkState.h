#ifndef RE_MINECRAFT_CHUNKSTATE_H
#define RE_MINECRAFT_CHUNKSTATE_H

// enum class ChunkState {
//     UNLOADED,
//     GENERATING,
//     GENERATED,
//     MESHING,
//     MESHED,
//     READY,
// };

enum class ChunkState
{
    UNLOADED,

    // Pass 1: Terrain
    TERRAIN_PENDING,
    TERRAIN_GENERATING,
    TERRAIN_DONE,

    // Pass 2: Decoration
    DECOR_PENDING,
    DECOR_GENERATING,
    DECOR_DONE,

    // Meshing
    MESHING,
    MESHED,
    READY
};

// Helper to check if terrain is complete
inline bool hasTerrainComplete(ChunkState state) {
    return state >= ChunkState::TERRAIN_DONE;
}

// Helper to check if fully generated
inline bool isFullyGenerated(ChunkState state) {
    return state >= ChunkState::DECOR_DONE;
}

#endif //RE_MINECRAFT_CHUNKSTATE_H