#ifndef RE_MINECRAFT_CHUNKMANAGER_H
#define RE_MINECRAFT_CHUNKMANAGER_H

#pragma once

class World; // forward declaration

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <iostream>

#include <glm/glm.hpp>

#include "ChunkPos.h"
#include "Chunk.h"
#include "Shader.h"

using ChunkMap = std::unordered_map<ChunkPos, std::unique_ptr<Chunk>, ChunkPosHash>;

class ChunkManager {
    static constexpr uint8_t VIEW_DISTANCE = 4;
    ChunkMap chunks;

    public:
        Chunk& getOrCreateChunk(int cx, int cy, int cz);
        [[nodiscard]] Chunk* getChunk(int cx, int cy, int cz) const;
        [[nodiscard]] const ChunkMap& getChunks() const;
        [[nodiscard]] bool chunkExist(int cx, int cy, int cz) const;
        void markNeighborsDirty(const ChunkPos& cp, const std::optional<BlockPos>& bp = std::nullopt) const;

        void update(const World& world, const glm::vec3& cameraWorldPos);
};

#endif //RE_MINECRAFT_CHUNKMANAGER_H