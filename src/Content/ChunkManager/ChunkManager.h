#ifndef RE_MINECRAFT_CHUNKMANAGER_H
#define RE_MINECRAFT_CHUNKMANAGER_H

#include <unordered_map>
#include <memory>

#include "ChunkPos.h"
#include "Chunk.h"
#include "Shader.h"

using ChunkMap = std::unordered_map<ChunkPos, std::unique_ptr<Chunk>, ChunkPosHash>;

class ChunkManager {
    ChunkMap chunks;

    public:
        Chunk& getOrCreateChunk(int cx, int cy, int cz);
        [[nodiscard]] Chunk* getChunk(int cx, int cy, int cz) const;
        [[nodiscard]] const ChunkMap& getChunks() const;
        [[nodiscard]] bool chunkExist(int cx, int cy, int cz) const;

        void markNeighborsDirty(const ChunkPos& cp, const std::optional<BlockPos>& bp = std::nullopt) const;
};

#endif //RE_MINECRAFT_CHUNKMANAGER_H