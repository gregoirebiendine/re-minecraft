#include "World.h"
#include "Viewport.h"
#include "Systems/MovementSystem.h"
#include "Systems/RenderSystem.h"
#include "Components/MeshRefComponent.h"

World::World(
    const BlockRegistry& _blockRegistry,
    const TextureRegistry& _textureRegistry,
    const PrefabRegistry& _prefabRegistry,
    const MeshRegistry& _meshRegistry
) :
    blockRegistry(_blockRegistry),
    textureRegistry(_textureRegistry),
    meshRegistry(_meshRegistry),
    shader("/resources/shaders/World/"),
    chunkManager(_blockRegistry, _prefabRegistry),
    meshManager(*this)
{
    // Setup entity vector
    this->entities.reserve(MAX_ENTITY);

    // Register MovementSystem to ECS
    this->scheduler.registerSystem<ECS::MovementSystem>();
    this->scheduler.registerSystem<ECS::RenderSystem>();

    // Create a Zombie entity with initial position and velocity
    const auto zombieMesh = this->meshRegistry.get("zombie");
    const auto zombieTexture = this->textureRegistry.getByName("zombie");
    const auto zombie = this->ecs.createEntity();
    this->ecs.addComponent(zombie, ECS::Position{8.5f, 71.f, 8.5f});
    this->ecs.addComponent(zombie, ECS::Velocity{0.f, 0.f, 0.f});
    this->ecs.addComponent(zombie, ECS::MeshRefComponent{ zombieMesh, zombieTexture });
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

void World::update(const Camera& camera, const float aspect, const glm::mat4& vpMatrix)
{
    const glm::vec3 cameraPosition = camera.getPosition();

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
    this->chunkManager.updateFrustum(vpMatrix);
    this->meshManager.scheduleMeshing(cameraPosition);
    this->meshManager.update();

    // Update render system
    auto& renderSystem = this->scheduler.getSystem<ECS::RenderSystem>();
    renderSystem.setProjectionMatrix(camera.getProjectionMatrix(aspect));
    renderSystem.setViewMatrix(camera.getViewMatrix());

    // Update ECS
    this->scheduler.update(this->ecs, Viewport::dt);
}

void World::render()
{
    this->shader.use();

    // Render block
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
