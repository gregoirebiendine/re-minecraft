#ifndef FARFIELD_CHUNKMESHMANAGER_H
#define FARFIELD_CHUNKMESHMANAGER_H

#pragma once

class World; // Forward declaration

#include <unordered_map>
#include <queue>
#include <mutex>

#include "Utils.h"
#include "ChunkNeighbors.h"
#include "ChunkManager.h"
#include "ChunkMesh.h"
#include "ThreadPool.h"

struct NeighborData {
    bool exists;
    BlockStorage blocks;
};

class ChunkMeshManager {
    public:
        explicit ChunkMeshManager(World& _world);

        void update();
        void requestRebuild(Chunk& chunk, float distance);
        void scheduleMeshing(const glm::vec3& playerPos);

        const ChunkMesh& getMesh(const ChunkPos& pos) const;

    private:
        static void buildFaceMesh(MeshData& mesh, const glm::ivec3& pos, MaterialFace face, uint16_t texId, BlockRotation rotation);
        static std::string getTextureFromRotation(const BlockMeta& meta, MaterialFace face, BlockRotation rotation);

        void buildMeshJob(const ChunkJob& job);
        bool isTransparentAtSnapshot(BlockId blockId) const;
        bool isAirAtSnapshot(const BlockStorage& blockData, const NeighborData neighbors[6], int x, int y, int z) const;

        World& world;
        ThreadPool<ChunkJob> workers;

        std::unordered_map<ChunkPos, ChunkMesh, ChunkPosHash> meshes;

        std::mutex uploadMutex;
        std::queue<std::pair<ChunkPos, MeshData>> uploadQueue;
};

#endif