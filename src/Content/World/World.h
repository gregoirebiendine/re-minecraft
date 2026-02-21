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
#include "ItemRegistry.h"
#include "Registries.h"
#include "Shader.h"
#include "ECS/ISystem.h"

class World {
    static constexpr ChunkPos SPAWN_CENTER{0, 4, 0};
    static constexpr int SPAWN_RADIUS = 3;
    static constexpr int MAX_ENTITY = 100;

    const Registries& registries;
    const InputState& inputs;

    Shader shader;
    ChunkManager chunkManager;
    ChunkMeshManager meshManager;

    ECS::Handler ecs{MAX_ENTITY};
    ECS::SystemScheduler scheduler{};

    std::vector<ECS::IEntity> entities{};
    ECS::IEntity player;

    bool isSimulationReady = false;

    public:
        explicit World(const Registries& _registries, const InputState& _inputs);

        // Get ECS members
        ECS::Handler& getECS() { return this->ecs; }
        ECS::SystemScheduler& getECSScheduler() { return this->scheduler; }
        ECS::IEntity& getPlayerEntity() { return this->player; }

        // Get other members
        const Registries& getRegistries() const { return this->registries; }
        ChunkManager& getChunkManager() { return this->chunkManager; }
        Shader& getShader() { return this->shader; }

        // Lifecycle
        void fill(glm::ivec3 from, glm::ivec3 to, Material mat);
        void setBlock(int wx, int wy, int wz, Material mat);
        void setBlock(int wx, int wy, int wz, const std::string& blockName);
        [[nodiscard]] Material getBlock(int wx, int wy, int wz);
        [[nodiscard]] Material getBlock(glm::ivec3 pos);
        [[nodiscard]] bool isAir(int wx, int wy, int wz);
        bool isEntityAt(glm::ivec3 blockPos);

        // Updates
        void update(float aspect);
        void render();
};

#endif
