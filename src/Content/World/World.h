#ifndef WORLD_H
#define WORLD_H

#include <unordered_map>
#include <memory>
#include <ranges>

#include <FastNoiseLite.h>

#include "BlockRegistry.h"
#include "TextureRegistry.h"
#include "ChunkMeshManager.h"
#include "Chunk.h"
#include "Shader.h"

class World {
    std::unordered_map<ChunkPos, std::unique_ptr<Chunk>, ChunkPosHash> chunks;
    std::vector<ChunkPos> dirtyChunks;
    ChunkMeshManager meshManager;
    BlockRegistry blockRegistry;
    TextureRegistry textureRegistry;
    FastNoiseLite noise;

    static ChunkPos blockToChunk(int wx, int wy, int wz);
    static BlockPos blockToLocal(int wx, int wy, int wz);

    public:
        explicit World(BlockRegistry _blockRegistry, const TextureRegistry& _textureRegistry);

        const BlockRegistry& getBlockRegistry() const;
        const TextureRegistry& getTextureRegistry() const;

        // Getters
        Chunk& getOrCreateChunk(int cx, int cy, int cz);
        [[nodiscard]] Chunk* getChunk(int cx, int cy, int cz);
        [[nodiscard]] bool chunkExist(int cx, int cy, int cz) const;
        [[nodiscard]] Material getBlock(int wx, int wy, int wz) const;
        [[nodiscard]] bool isAir(int wx, int wy, int wz) const;

        // Terrain
        int getTerrainHeight(int worldX, int worldZ) const;
        void generateChunkTerrain(Chunk& chunk) const;

        // Setters
        void setBlock(int wx, int wy, int wz, Material id);
        void fill(glm::ivec3 from, glm::ivec3 to, Material id);
        void markNeighborsDirty(const ChunkPos& cp, const std::optional<BlockPos>& bp = std::nullopt);

        void update();
        void render(const Shader& worldShader);
};

#endif //WORLD_H
