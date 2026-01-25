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
    BlockRegistry blockRegistry;
    TextureRegistry textureRegistry;

    Shader shader;
    ChunkManager chunkManager;
    ChunkMeshManager meshManager;

    public:
        explicit World(BlockRegistry _blockRegistry, const TextureRegistry& _textureRegistry);

        const BlockRegistry& getBlockRegistry() const;
        const TextureRegistry& getTextureRegistry() const;
        const Shader& getShader() const;
        ChunkManager& getChunkManager();

        // Lifecycle
        void fill(glm::ivec3 from, glm::ivec3 to, Material mat);
        void setBlock(int wx, int wy, int wz, Material mat);
        void setBlock(int wx, int wy, int wz, const std::string& blockName);
        [[nodiscard]] Material getBlock(int wx, int wy, int wz);
        [[nodiscard]] bool isAir(int wx, int wy, int wz);

        // Updates
        void update(const glm::vec3& cameraPos, const glm::mat4& vpMatrix);
        void render();
};

#endif //WORLD_H
