#ifndef FARFIELD_CHUNKMANAGER_H
#define FARFIELD_CHUNKMANAGER_H

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
#include "PrefabRegistry.h"
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
    public:
        explicit ChunkManager(const BlockRegistry& _blockRegistry, const PrefabRegistry& _prefabRegistry);

        [[nodiscard]] std::shared_lock<std::shared_mutex> acquireReadLock() const;
        [[nodiscard]] ChunkMap& getChunks();
        [[nodiscard]] std::vector<Chunk*> getRenderableChunks();

        [[nodiscard]] Chunk* getChunk(int cx, int cy, int cz);
        bool canDecorate(const ChunkPos& pos);

        [[nodiscard]] ChunkNeighbors getNeighbors(const ChunkPos &cp);
        void rebuildNeighbors(const ChunkPos& pos);

        void setViewDistance(uint8_t dist);

        void updateStreaming(const glm::vec3& cameraPos);
        void updateFrustum(const glm::mat4& vpMatrix);
        void requestChunk(const ChunkPos& pos);

    private:
        const BlockRegistry& blockRegistry;

        std::unordered_map<ChunkPos, std::unique_ptr<Chunk>, ChunkPosHash> chunks;
        mutable std::shared_mutex chunksMutex;

        ThreadPool<ChunkJob> terrainWorkers;
        ThreadPool<ChunkJob> decorationWorkers;

        Frustum frustum{};
        TerrainGenerator terrainGenerator;

        uint8_t viewDistance{8};
        uint8_t unloadDistance{10};

        // Decoration locking mechanism to prevent concurrent writes to the same chunks
        std::unordered_set<ChunkPos, ChunkPosHash> decorationLocks;
        std::mutex decorationLockMutex;

        // Job handlers
        void terrainJob(const ChunkJob& job);
        void decorationJob(const ChunkJob& job);

        // Check and queue chunks ready for decoration
        void updateDecorationQueue();
        void tryQueueDecoration(const ChunkPos& pos);

        // Decoration lock management
        bool tryAcquireDecorationLock(const ChunkPos& pos);
        void releaseDecorationLock(const ChunkPos& pos);
};

#endif