#ifndef RE_MINECRAFT_CHUNKMESHMANAGER_H
#define RE_MINECRAFT_CHUNKMESHMANAGER_H

#include <unordered_map>
#include <memory>

#include <ChunkMesh.h>

class ChunkMeshManager {
    public:
        ChunkMesh& get(const Chunk& chunk);
        void remove(const ChunkPos& pos);

    private:
        std::unordered_map<ChunkPos, std::unique_ptr<ChunkMesh>, ChunkPosHash> meshes{};
};

#endif //RE_MINECRAFT_CHUNKMESHMANAGER_H