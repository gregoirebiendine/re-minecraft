#ifndef RE_MINECRAFT_CHUNKMANAGER_H
#define RE_MINECRAFT_CHUNKMANAGER_H

#include <thread>
#include <condition_variable>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <memory>
#include <ranges>
#include <iostream>
#include <utility>
#include <shared_mutex>

#include <glm/glm.hpp>

#include "TerrainGenerator.h"
#include "ThreadPool.h"
#include "ChunkPos.h"
#include "Chunk.h"
#include "ChunkNeighbors.h"
#include "Frustum.h"

using ChunkMap = std::unordered_map<ChunkPos, std::unique_ptr<Chunk>, ChunkPosHash>;

struct ChunkJob {
    ChunkPos pos;
    float distance;
    uint64_t generationID;

    bool operator<(const ChunkJob& other) const {
        return distance > other.distance;
    }
};

class ChunkManager {
    const BlockRegistry& blockRegistry;

    std::unordered_map<ChunkPos, std::unique_ptr<Chunk>, ChunkPosHash> chunks;
    mutable std::shared_mutex chunksMutex;
    ThreadPool<ChunkJob> workers;

    Frustum frustum{};
    TerrainGenerator terrainGenerator;

    uint8_t viewDistance{8};
    uint8_t unloadDistance{10};

    void generateJob(const ChunkJob& job);

    public:
        explicit ChunkManager(const BlockRegistry& _blockRegistry);

        [[nodiscard]] std::shared_lock<std::shared_mutex> acquireReadLock() const;
        [[nodiscard]] ChunkMap& getChunks();
        [[nodiscard]] Chunk* getChunk(int cx, int cy, int cz);
        [[nodiscard]] std::vector<Chunk*> getRenderableChunks();
        [[nodiscard]] ChunkNeighbors getNeighbors(const ChunkPos &cp);
        void rebuildNeighbors(const ChunkPos& pos);

        void setViewDistance(unsigned char dist);

        void updateStreaming(const glm::vec3& cameraPos);
        void updateFrustum(const glm::mat4& vpMatrix);
        void requestChunk(const ChunkPos& pos);
};

#endif //RE_MINECRAFT_CHUNKMANAGER_H