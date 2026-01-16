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

#include <glm/glm.hpp>

#include "TerrainGenerator.h"
#include "ThreadPool.h"
#include "ChunkPos.h"
#include "Chunk.h"
#include "ChunkNeighbors.h"

using ChunkMap = std::unordered_map<ChunkPos, Chunk, ChunkPosHash>;

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

    std::unordered_map<ChunkPos, Chunk, ChunkPosHash> chunks;
    ThreadPool<ChunkJob> workers;
    BlockRegistry blockRegistry;

    void generateJob(ChunkJob job);

    public:
        explicit ChunkManager(BlockRegistry _blockRegistry);

        [[nodiscard]] ChunkMap& getChunks();
        [[nodiscard]] Chunk* getChunk(int cx, int cy, int cz);
        std::vector<Chunk*> getRenderableChunks();
        [[nodiscard]] ChunkNeighbors getNeighbors(const ChunkPos &cp);
        void rebuildNeighbors(const ChunkPos& pos);

        void updateStreaming(const glm::vec3& cameraPos);
        void requestChunk(const ChunkPos& pos);
};

#endif //RE_MINECRAFT_CHUNKMANAGER_H