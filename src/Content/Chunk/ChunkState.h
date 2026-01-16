#ifndef RE_MINECRAFT_CHUNKSTATE_H
#define RE_MINECRAFT_CHUNKSTATE_H

enum class ChunkState {
    UNLOADED,       // Unloaded,
    GENERATING,     // Generating,
    GENERATED,      // Ready,
    MESHING,        // Meshing,
    MESHED,         // MeshReady,
    READY,          // Uploaded
};

#endif //RE_MINECRAFT_CHUNKSTATE_H