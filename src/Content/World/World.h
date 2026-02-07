#ifndef WORLD_H
#define WORLD_H

#include <unordered_map>
#include <memory>
#include <ranges>
#include <utility>

#include "glm/glm.hpp"

#include "Camera.h"
#include "BlockRegistry.h"
#include "TextureRegistry.h"
#include "PrefabRegistry.h"
#include "MeshRegistry.h"
#include "ChunkManager.h"
#include "ChunkMeshManager.h"
#include "Shader.h"
#include "Viewport.h"
#include "ECS/Systems/MovementSystem.h"
#include "ECS/Systems/RenderSystem.h"

class World {
    static constexpr int MAX_ENTITY = 100;

    const BlockRegistry& blockRegistry;
    const TextureRegistry& textureRegistry;
    const MeshRegistry& meshRegistry;

    Shader shader;
    ChunkManager chunkManager;
    ChunkMeshManager meshManager;

    ECS::Handler ecs{MAX_ENTITY};
    ECS::SystemScheduler scheduler;
    std::vector<ECS::Entity> entities;

    glm::mat4 cachedProjection{1.0f};
    glm::mat4 cachedView{1.0f};

    public:
        explicit World(
            const BlockRegistry& _blockRegistry,
            const TextureRegistry& _textureRegistry,
            const PrefabRegistry& _prefabRegistry,
            const MeshRegistry& _meshRegistry
        );

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
        void update(const Camera& camera, float aspect, const glm::mat4& vpMatrix);
        void render();
};

#endif //WORLD_H
