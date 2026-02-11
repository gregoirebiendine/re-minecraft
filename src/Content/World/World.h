#ifndef FARFIELD_WORLD_H
#define FARFIELD_WORLD_H

#include <unordered_map>
#include <memory>
#include <ranges>
#include <utility>

#include "glm/glm.hpp"

#include "BlockRegistry.h"
#include "TextureRegistry.h"
#include "PrefabRegistry.h"
#include "MeshRegistry.h"
#include "ChunkManager.h"
#include "ChunkMeshManager.h"
#include "InputState.h"
#include "Shader.h"
#include "ECS/ISystem.h"

class World {
    static constexpr int MAX_ENTITY = 100;

    const BlockRegistry& blockRegistry;
    const TextureRegistry& textureRegistry;
    const MeshRegistry& meshRegistry;
    const InputState& inputs;

    Shader shader;
    ChunkManager chunkManager;
    ChunkMeshManager meshManager;

    ECS::Handler ecs{MAX_ENTITY};
    ECS::SystemScheduler scheduler{};

    std::vector<ECS::IEntity> entities{};
    ECS::IEntity player;

    public:
        explicit World(
            const BlockRegistry& _blockRegistry,
            const TextureRegistry& _textureRegistry,
            const PrefabRegistry& _prefabRegistry,
            const MeshRegistry& _meshRegistry,
            const InputState& _inputs
        );

        ECS::Handler& getECS() { return this->ecs; }
        ECS::SystemScheduler& getECSScheduler() { return this->scheduler; }
        ECS::IEntity& getPlayerEntity() { return this->player; }

        const BlockRegistry& getBlockRegistry() const;
        const TextureRegistry& getTextureRegistry() const;
        Shader& getShader();
        ChunkManager& getChunkManager();

        // Lifecycle
        void fill(glm::ivec3 from, glm::ivec3 to, Material mat);
        void setBlock(int wx, int wy, int wz, Material mat);
        void setBlock(int wx, int wy, int wz, const std::string& blockName);
        [[nodiscard]] Material getBlock(int wx, int wy, int wz);
        [[nodiscard]] bool isAir(int wx, int wy, int wz);

        // Updates
        void update(float aspect);
        void render();
};

#endif //WORLD_H
