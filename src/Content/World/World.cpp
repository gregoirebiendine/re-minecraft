#include "World.h"
#include "Viewport.h"

#include "Systems/PlayerMovementSystem.h"
#include "Systems/PlayerInputSystem.h"
#include "Systems/CameraSystem.h"
#include "Systems/RenderSystem.h"
#include "Systems/GravitySystem.h"
#include "Systems/FacingSystem.h"
#include "Systems/MovementSystem.h"
#include "Systems/CollisionSystem.h"

#include "Components/Movements.h"
#include "Components/Gravity.h"
#include "Components/CollisionBox.h"
#include "Components/MeshRef.h"
#include "Components/PlayerInput.h"
#include "Components/Friction.h"

World::World(
    const BlockRegistry& _blockRegistry,
    const TextureRegistry& _textureRegistry,
    const PrefabRegistry& _prefabRegistry,
    const MeshRegistry& _meshRegistry,
    const InputState& _inputs
) :
    blockRegistry(_blockRegistry),
    textureRegistry(_textureRegistry),
    meshRegistry(_meshRegistry),
    inputs(_inputs),
    shader("/resources/shaders/World/"),
    chunkManager(_blockRegistry, _prefabRegistry),
    meshManager(*this)
{
    // Setup entity vector
    this->entities.reserve(MAX_ENTITY);

    // Register MovementSystem to ECS
    this->scheduler.registerSystem<ECS::PlayerInputSystem>(this->inputs);
    this->scheduler.registerSystem<ECS::CameraSystem>(this->inputs);
    this->scheduler.registerSystem<ECS::GravitySystem>();
    this->scheduler.registerSystem<ECS::PlayerMovementSystem>();
    this->scheduler.registerSystem<ECS::FacingSystem>();
    this->scheduler.registerSystem<ECS::MovementSystem>();
    this->scheduler.registerSystem<ECS::CollisionSystem>(*this);
    this->scheduler.registerSystem<ECS::RenderSystem>();

    // Create player entity
    // const auto playerMesh = this->meshRegistry.get("player");
    // const auto playerTexture = this->textureRegistry.getByName("player");
    this->player = this->ecs.createEntity();
    this->ecs.addComponent(this->player, ECS::Position{8.5f, 73.f, 8.5f});
    this->ecs.addComponent(this->player, ECS::Velocity());
    this->ecs.addComponent(this->player, ECS::Rotation());
    this->ecs.addComponent(this->player, ECS::Camera{});
    this->ecs.addComponent(this->player, ECS::PlayerInput{});
    this->ecs.addComponent(this->player, ECS::Gravity{});
    this->ecs.addComponent(this->player, ECS::Friction{});
    this->ecs.addComponent(this->player, ECS::CollisionBox{{0.3f, 0.9f, 0.3f}});
    // this->ecs.addComponent(this->player, ECS::MeshRef{ playerMesh, playerTexture });

    // Create a Zombie entity
    const auto zombieMesh = this->meshRegistry.get("zombie");
    const auto zombieTexture = this->textureRegistry.getByName("zombie");
    const auto zombie = this->ecs.createEntity();
    this->ecs.addComponent(zombie, ECS::Position{7.5f, 73.f, 7.5f});
    this->ecs.addComponent(zombie, ECS::Rotation());
    this->ecs.addComponent(zombie, ECS::Velocity());
    this->ecs.addComponent(zombie, ECS::Gravity());
    this->ecs.addComponent(zombie, ECS::CollisionBox{{0.45f, 1.f, 0.45f}});
    this->ecs.addComponent(zombie, ECS::MeshRef{ zombieMesh, zombieTexture });
    this->entities.emplace_back(zombie);

    // Set WorldShader uniform to use loaded textures
    this->shader.use();
    this->shader.setUniformInt("Textures", 0);

    // Create spawn area
    constexpr glm::ivec2 rx = {SPAWN_CENTER.x - SPAWN_RADIUS, SPAWN_CENTER.x + SPAWN_RADIUS};
    constexpr glm::ivec2 ry = {SPAWN_CENTER.y - SPAWN_RADIUS, SPAWN_CENTER.y + SPAWN_RADIUS};
    constexpr glm::ivec2 rz = {SPAWN_CENTER.z - SPAWN_RADIUS, SPAWN_CENTER.z + SPAWN_RADIUS};

    for (int z = rz.x; z <= rz.y; ++z)
        for (int y = ry.x; y <= ry.y; ++y)
            for (int x = rx.x; x <= rx.y; ++x)
                this->chunkManager.requestChunk({x, y, z});
}


Material World::getBlock(const int wx, const int wy, const int wz)
{
    const auto [cx, cy, cz] = ChunkPos::fromWorld(wx, wy, wz);
    const Chunk* chunk = this->chunkManager.getChunk(cx, cy, cz);

    if (!chunk)
        return this->blockRegistry.getByName("core:air");

    const auto [x, y, z] = BlockPos::fromWorld(wx, wy, wz);
    return chunk->getBlock(x, y, z);
}

Material World::getBlock(const glm::ivec3 pos)
{
    return this->getBlock(pos.x, pos.y, pos.z);
}

bool World::isAir(const int wx, const int wy, const int wz)
{
    const auto mat = this->getBlock(wx, wy, wz);
    return this->blockRegistry.isEqual(BlockData::getBlockId(mat), "core:air");
}

void World::setBlock(const int wx, const int wy, const int wz, const Material mat)
{
    const auto [cx, cy, cz] = ChunkPos::fromWorld(wx, wy, wz);
    const auto [x, y, z] = BlockPos::fromWorld(wx, wy, wz);

    Chunk* chunk = this->chunkManager.getChunk(cx, cy, cz);

    if (!chunk || chunk->getState() != ChunkState::READY)
        return;

    chunk->setBlock(x, y, z, mat);
    chunk->setDirty(true);

    this->chunkManager.rebuildNeighbors({cx, cy, cz});
}

void World::setBlock(const int wx, const int wy, const int wz, const std::string& blockName)
{
    const BlockId id = this->blockRegistry.getByName(blockName);

    this->setBlock(wx, wy, wz, BlockData::packBlockData(id, 0));
}

void World::fill(const glm::ivec3 from, const glm::ivec3 to, const Material mat)
{
    for (int z = from.z; z <= to.z; ++z)
        for (int y = from.y; y <= to.y; ++y)
            for (int x = from.x; x <= to.x; ++x)
            {
                this->setBlock(x, y, z, mat);
            }
}

bool World::isEntityAt(const glm::ivec3 blockPos)
{
    const auto blockMin = glm::vec3(blockPos);
    const glm::vec3 blockMax = blockMin + 1.0f;

    bool found = false;

    auto view = this->ecs.query<ECS::Position, ECS::CollisionBox>();
    auto& rotPool = this->ecs.getPool<ECS::Rotation>();

    view.forEach([&]([[maybe_unused]] ECS::EntityId id, const ECS::Position& pos, const ECS::CollisionBox& box) {
        const float yaw = rotPool.has(id) ? rotPool.get(id).y : 0.0f;
        const glm::vec3 halfExt = ECS::computeRotatedHalfExtents(box.halfExtents, yaw);
        const ECS::AABB aabb = ECS::computeAABB(pos, halfExt);

        if (blockMin.x < aabb.max.x && blockMax.x > aabb.min.x &&
            blockMin.y < aabb.max.y && blockMax.y > aabb.min.y &&
            blockMin.z < aabb.max.z && blockMax.z > aabb.min.z)
        {
            found = true;
        }
    });

    return found;
}


// Update and render
void World::update(const float aspect)
{
    // Get systems
    auto& cameraSystem = this->scheduler.getSystem<ECS::CameraSystem>();
    auto& renderSystem = this->scheduler.getSystem<ECS::RenderSystem>();

    // Update window aspect inside camera system
    cameraSystem.setAspect(aspect);

    // Update ECS
    if (this->isSimulationReady)
        this->scheduler.update(this->ecs, Viewport::dt);
    else
        this->isSimulationReady = this->chunkManager.isAreaReady(SPAWN_CENTER, SPAWN_RADIUS);

    // Get ViewMatrix and ProjectionMatrix
    const auto& v = cameraSystem.getViewMatrix();
    const auto& p = cameraSystem.getProjectionMatrix();
    const auto& playerPos = this->ecs.getComponent<ECS::Position>(this->player);

    // Update shaders matrices
    renderSystem.setProjectionMatrix(p);
    renderSystem.setViewMatrix(v);
    this->shader.setProjectionMatrix(p);
    this->shader.setViewMatrix(v);

    // Swap chunks pending changes
    {
        auto lock = chunkManager.acquireReadLock();
        for (auto& chunk : chunkManager.getChunks() | std::views::values) {
            if (chunk->hasPendingChanges())
                chunk->swapBuffers();
        }
    }

    // Update chunk meshing
    this->chunkManager.updateStreaming(playerPos);
    this->chunkManager.updateFrustum(p * v);
    this->meshManager.scheduleMeshing(playerPos);
    this->meshManager.update();
}

void World::render()
{
    this->shader.use();

    // Render chunks
    for (const auto chunk : this->chunkManager.getRenderableChunks()) {
        const auto& mesh = this->meshManager.getMesh(chunk->getPosition());

        this->shader.setModelMatrix(chunk->getChunkModel());
        mesh.render();
    }

    // Render ECS
    this->scheduler.render(this->ecs);
}


// Others
const BlockRegistry& World::getBlockRegistry() const
{
    return this->blockRegistry;
}

const TextureRegistry& World::getTextureRegistry() const
{
    return this->textureRegistry;
}

ChunkManager& World::getChunkManager()
{
    return this->chunkManager;
}

Shader& World::getShader()
{
    return this->shader;
}
