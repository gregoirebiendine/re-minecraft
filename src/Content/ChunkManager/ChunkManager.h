#ifndef RE_MINECRAFT_CHUNKMANAGER_H
#define RE_MINECRAFT_CHUNKMANAGER_H

#include <thread>
#include <condition_variable>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <memory>
#include <iostream>

#include <glm/glm.hpp>

#include "TerrainGenerator.h"
#include "ThreadPool.h"
#include "ChunkPos.h"
#include "Chunk.h"
#include "ChunkNeighbors.h"

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
    static constexpr uint8_t VIEW_DISTANCE = 8;

    ChunkMap chunks;
    ThreadPool<ChunkJob> workers;

    void generateJob(ChunkJob job);
    void rebuildNeighbors(const ChunkPos& pos) const;

    public:
        ChunkManager();

        [[nodiscard]] const ChunkMap& getChunks() const;
        [[nodiscard]] ChunkNeighbors getNeighbors(const ChunkPos &cp) const;

        void updateStreaming(const glm::vec3& cameraPos);
        void requestChunk(const ChunkPos& pos);

        [[nodiscard]] Chunk* getChunk(int cx, int cy, int cz) const;
        Chunk& getOrCreateChunk(int cx, int cy, int cz);
        void markNeighborsDirty(const ChunkPos& cp, const std::optional<BlockPos>& bp = std::nullopt) const;
};

#endif //RE_MINECRAFT_CHUNKMANAGER_H