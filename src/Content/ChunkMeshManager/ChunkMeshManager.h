#ifndef RE_MINECRAFT_CHUNKMESHMANAGER_H
#define RE_MINECRAFT_CHUNKMESHMANAGER_H

#include <unordered_map>
#include <memory>
#include <thread>
#include <ranges>

#include "ConcurrentQueue.h"
#include "ChunkMesh.h"
#include "ChunkManager.h"
#include "ChunkNeighbors.h"
#include "ThreadPool.h"
#include "Utils.h"

class ChunkMeshManager {
    public:
        explicit ChunkMeshManager(World& _world);

        // static ChunkMeshData buildMesh(
        //     const Chunk& chunk,
        //     const ChunkNeighbors& n,
        //     const BlockRegistry& blockRegistry,
        //     const TextureRegistry& textureRegistry
        // );

        void update();
        void requestRebuild(Chunk&);

    private:
        void buildJob(ChunkPos pos);

        World& world;
        ThreadPool workers;

        std::unordered_map<ChunkPos, ChunkMesh, ChunkPosHash> meshes;

        std::mutex uploadMutex;
        std::queue<std::pair<ChunkPos, ChunkMeshData>> uploadQueue;

        // static void buildFaceMesh(
        //     ChunkMeshData& data,
        //     const glm::ivec3& v0,
        //     const glm::ivec3& v1,
        //     const glm::ivec3& v2,
        //     const glm::ivec3& v3,
        //     const glm::ivec3& normals,
        //     const uint16_t& texId
        // );

        static bool isAirAt(const Chunk& chunk, const ChunkNeighbors& n, int x, int y, int z);
};

#endif //RE_MINECRAFT_CHUNKMESHMANAGER_H