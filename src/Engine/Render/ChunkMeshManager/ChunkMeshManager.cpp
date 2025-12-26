#include "ChunkMeshManager.h"

ChunkMesh& ChunkMeshManager::get(const Chunk& chunk)
{
    ChunkPos pos = chunk.getPosition();
    const auto it = meshes.find(pos);

    if (it == meshes.end()) {
        auto [newIt, _] =
            meshes.emplace(pos, std::make_unique<ChunkMesh>());
        return *newIt->second;
    }

    return *it->second;
}

void ChunkMeshManager::remove(const ChunkPos& pos)
{
    meshes.erase(pos);
}