#ifndef WORLD_H
#define WORLD_H

#include <unordered_map>
#include <memory>
#include <ranges>
#include <utility>

#include <FastNoiseLite.h>

#include "BlockRegistry.h"
#include "TextureRegistry.h"
#include "ChunkManager.h"
#include "ChunkMeshManager.h"
#include "Chunk.h"
#include "Shader.h"
#include "Camera.h"

class World {
    std::unique_ptr<ChunkManager> chunkManager;
    ChunkMeshManager meshManager;

    BlockRegistry blockRegistry;
    TextureRegistry textureRegistry;
    FastNoiseLite noise;

    static BlockPos worldPosToLocalPos(int wx, int wy, int wz);

    public:
        explicit World(BlockRegistry _blockRegistry, const TextureRegistry& _textureRegistry);

        const BlockRegistry& getBlockRegistry() const;
        const TextureRegistry& getTextureRegistry() const;

        // Lifecycle
        void fill(glm::ivec3 from, glm::ivec3 to, Material id) const;
        void setBlock(int wx, int wy, int wz, Material id) const;
        [[nodiscard]] Material getBlock(int wx, int wy, int wz) const;
        [[nodiscard]] bool isAir(int wx, int wy, int wz) const;

        // Terrain
        int getTerrainHeight(int worldX, int worldZ) const;
        void generateChunkTerrain(Chunk& chunk) const;

        // Updates
        void update(const glm::vec3& cameraPos);
        void render(const Shader& worldShader);
};

#endif //WORLD_H
