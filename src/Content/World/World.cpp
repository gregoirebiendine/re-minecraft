#include "World.h"
#include "Viewport.h"

#include "Systems/PlayerMovementSystem.h"
#include "Systems/PlayerInputSystem.h"
#include "Systems/CameraSystem.h"
#include "Systems/RenderSystem.h"
#include "Systems/GravitySystem.h"
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
    this->scheduler.registerSystem<ECS::CollisionSystem>(*this);
    this->scheduler.registerSystem<ECS::RenderSystem>();

    // Create player entity
    this->player = this->ecs.createEntity();
    this->ecs.addComponent(this->player, ECS::Position{8.5f, 71.f, 8.5f});
    this->ecs.addComponent(this->player, ECS::Velocity{0.f, 0.f, 0.f});
    this->ecs.addComponent(this->player, ECS::Camera{});
    this->ecs.addComponent(this->player, ECS::PlayerInput{});
    this->ecs.addComponent(this->player, ECS::Gravity{});
    this->ecs.addComponent(this->player, ECS::Friction{});
    this->ecs.addComponent(this->player, ECS::CollisionBox{{0.3f, 0.9f, 0.3f}});

    // Create a Zombie entity
    const auto zombieMesh = this->meshRegistry.get("zombie");
    const auto zombieTexture = this->textureRegistry.getByName("zombie");
    const auto zombie = this->ecs.createEntity();
    this->ecs.addComponent(zombie, ECS::Position{7.5f, 72.f, 7.5f});
    this->ecs.addComponent(zombie, ECS::Velocity{0.f, 0.f, 0.f});
    this->ecs.addComponent(zombie, ECS::Gravity());
    this->ecs.addComponent(zombie, ECS::CollisionBox{{0.475f, 1.f, 0.475f}});
    this->ecs.addComponent(zombie, ECS::MeshRef{ zombieMesh, zombieTexture });
    this->entities.emplace_back(zombie);

    // Set WorldShader uniform to use loaded textures
    this->shader.use();
    this->shader.setUniformInt("Textures", 0);

    // Create spawn area (8*8*5)
    for (int z = -4; z <= 4; ++z)
        for (int y = 0; y <= 5; ++y)
            for (int x = -4; x <= 4; ++x)
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

bool World::isAir(const int wx, const int wy, const int wz)
{
    return this->blockRegistry.isEqual(this->getBlock(wx, wy, wz), "core:air");
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


// Update and render
void World::update(const float aspect)
{
    // Get systems
    auto& cameraSystem = this->scheduler.getSystem<ECS::CameraSystem>();
    auto& renderSystem = this->scheduler.getSystem<ECS::RenderSystem>();

    // Update window aspect inside camera system
    cameraSystem.setAspect(aspect);

    // Update ECS
    this->scheduler.update(this->ecs, Viewport::dt);

    // Get ViewMatrix and ProjectionMatrix
    const auto& v = cameraSystem.getViewMatrix();
    const auto& p = cameraSystem.getProjectionMatrix();
    const auto& cameraPosition = cameraSystem.getCameraPosition();

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
    this->chunkManager.updateStreaming(cameraPosition);
    this->chunkManager.updateFrustum(p * v);
    this->meshManager.scheduleMeshing(cameraPosition);
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
