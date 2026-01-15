#ifndef RE_MINECRAFT_CHUNKMESHMANAGER_H
#define RE_MINECRAFT_CHUNKMESHMANAGER_H

#pragma once

#include <unordered_map>
#include <queue>
#include <mutex>

#include "Utils.h"
#include "ChunkMesh.h"
#include "ThreadPool.h"
#include "World.h"

class ChunkMeshManager {
    public:
        explicit ChunkMeshManager(World& world);

        void update();
        void requestRebuild(Chunk& chunk, float distance);

    private:
        static bool isAirAt(const Chunk& c, const ChunkNeighbors& n, int x, int y, int z);
        static void buildFaceMesh(
            MeshData& data,
            const glm::ivec3 &v0,
            const glm::ivec3 &v1,
            const glm::ivec3 &v2,
            const glm::ivec3 &v3,
            const glm::ivec3 &normals,
            const uint16_t& texId
        );
        void buildMeshJob(const ChunkJob& job);

        World& world;
        ThreadPool<ChunkJob> workers;

        std::unordered_map<ChunkPos, ChunkMesh, ChunkPosHash> meshes;

        std::mutex uploadMutex;
        std::queue<std::pair<ChunkPos, MeshData>> uploadQueue;
};

#endif //RE_MINECRAFT_CHUNKMESHMANAGER_H