#ifndef WORLD_H
#define WORLD_H

#include <unordered_map>
#include <memory>
#include <ranges>

#include <FastNoiseLite.h>

#include "BlockRegistry.h"
#include "ChunkMeshManager.h"
#include "Chunk.h"
#include "Shader.h"

class World {
    std::unordered_map<ChunkPos, std::unique_ptr<Chunk>, ChunkPosHash> chunks;
    std::vector<ChunkPos> dirtyChunks;
    ChunkMeshManager meshManager;
    BlockRegistry blockRegistry;

    static ChunkPos blockToChunk(int wx, int wy, int wz);
    static BlockPos blockToLocal(int wx, int wy, int wz);

    public:
        explicit World(BlockRegistry blockRegistry);

        // Getters
        Chunk& getOrCreateChunk(int cx, int cy, int cz);
        [[nodiscard]] Chunk* getChunk(int cx, int cy, int cz);
        [[nodiscard]] Material getBlock(int wx, int wy, int wz) const;
        [[nodiscard]] bool isAir(int wx, int wy, int wz) const;
        [[nodiscard]] bool chunkExist(int cx, int cy, int cz) const;

        // Terrain
        static int getTerrainHeight(int worldX, int worldZ, const FastNoiseLite &noise);
        void generateChunkTerrain(Chunk& chunk, const FastNoiseLite &noise) const;

        // Setters
        void setBlock(int wx, int wy, int wz, Material id);
        void fill(glm::ivec3 from, glm::ivec3 to, Material id);
        void markNeighborsDirty(const ChunkPos& cp, const std::optional<BlockPos>& bp = std::nullopt);
        // void markNeighborsDirty(const ChunkPos& cp);

        void update();
        void render(const Shader& shaders);
};

#endif //WORLD_H
