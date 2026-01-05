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
    ChunkMeshManager meshManager;
    BlockRegistry blockRegistry;

    static int mod(int a, int b);
    static ChunkPos worldToChunk(int wx, int wy, int wz);
    static int floorDiv(int a, int b);

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
        void markNeighborsDirty(const ChunkPos& cp);

        void render(const Shader& shaders);
};

#endif //WORLD_H
