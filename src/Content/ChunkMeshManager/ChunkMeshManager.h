#ifndef RE_MINECRAFT_CHUNKMESHMANAGER_H
#define RE_MINECRAFT_CHUNKMESHMANAGER_H

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
    std::array<Material, Chunk::VOLUME> blocks;
};

class ChunkMeshManager {
    public:
        explicit ChunkMeshManager(World& _world);

        void update();
        void requestRebuild(Chunk& chunk, float distance);
        void scheduleMeshing(const glm::vec3& cameraPos);

        const ChunkMesh& getMesh(const ChunkPos& pos) const;

    private:
        static bool isAirAtSnapshot(const std::array<Material, Chunk::VOLUME>& blockData, const NeighborData neighbors[6], int x, int y, int z);
        static void buildFaceMesh(MeshData& mesh, const glm::ivec3& pos, MaterialFace face, uint16_t texId, uint8_t rotation = 0);

        void buildMeshJob(const ChunkJob& job);

        World& world;
        ThreadPool<ChunkJob> workers;

        std::unordered_map<ChunkPos, ChunkMesh, ChunkPosHash> meshes;

        std::mutex uploadMutex;
        std::queue<std::pair<ChunkPos, MeshData>> uploadQueue;
};

#endif //RE_MINECRAFT_CHUNKMESHMANAGER_H