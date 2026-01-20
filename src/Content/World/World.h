#ifndef WORLD_H
#define WORLD_H

#include <unordered_map>
#include <memory>
#include <ranges>
#include <utility>

#include "glm/glm.hpp"

#include "BlockRegistry.h"
#include "TextureRegistry.h"
#include "ChunkManager.h"
#include "ChunkMeshManager.h"
#include "Shader.h"

class World {
    std::unique_ptr<Shader> shader;

    std::unique_ptr<ChunkManager> chunkManager;
    ChunkMeshManager meshManager;

    BlockRegistry blockRegistry;
    TextureRegistry textureRegistry;

    public:
        explicit World(BlockRegistry _blockRegistry, const TextureRegistry& _textureRegistry);

        const BlockRegistry& getBlockRegistry() const;
        const TextureRegistry& getTextureRegistry() const;
        const std::unique_ptr<ChunkManager>& getChunkManager() const;
        const Shader& getShader() const;

        // Lifecycle
        void fill(glm::ivec3 from, glm::ivec3 to, Material id) const;
        void setBlock(int wx, int wy, int wz, Material id) const;
        [[nodiscard]] Material getBlock(int wx, int wy, int wz) const;
        [[nodiscard]] bool isAir(int wx, int wy, int wz) const;

        // Updates
        void update(const glm::vec3& cameraPos);
        void render() const;
};

#endif //WORLD_H
