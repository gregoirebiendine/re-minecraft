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

#include "ConcurrentQueue.h"
#include "TerrainGenerator.h"
#include "ChunkPos.h"
#include "Chunk.h"
#include "ChunkNeighbors.h"

using ChunkMap = std::unordered_map<ChunkPos, std::unique_ptr<Chunk>, ChunkPosHash>;

class ChunkManager {
    static constexpr uint8_t VIEW_DISTANCE = 8;

    ChunkMap chunks;
    ConcurrentQueue<Chunk*> generationQueue;
    std::jthread worker;

    TerrainGenerator terrainGenerator;

    public:
        explicit ChunkManager(const TerrainGenerator& _terrainGenerator);

        Chunk& getOrCreateChunk(int cx, int cy, int cz);
        [[nodiscard]] Chunk* getChunk(int cx, int cy, int cz) const;
        [[nodiscard]] const ChunkMap& getChunks() const;
        [[nodiscard]] bool chunkExist(int cx, int cy, int cz) const;
        void markNeighborsDirty(const ChunkPos& cp, const std::optional<BlockPos>& bp = std::nullopt) const;
        ChunkNeighbors getNeighbors(const ChunkPos &cp) const;

        [[noreturn]] void workerLoop();
        void update(const glm::vec3& cameraPos);
};

#endif //RE_MINECRAFT_CHUNKMANAGER_H