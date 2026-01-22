#include "World.h"

World::World(BlockRegistry _blockRegistry, const TextureRegistry& _textureRegistry) :
    blockRegistry(std::move(_blockRegistry)),
    textureRegistry(_textureRegistry),
    shader(
    "../resources/shaders/World/world.vert",
    "../resources/shaders/World/world.frag"
    ),
    chunkManager(blockRegistry),
    meshManager(*this)
{
    this->shader.use();
    this->shader.setUniformInt("Textures", 0);

    TerrainGenerator::init();

    constexpr ChunkPos center{0,0,0};
    for (int z = -4; z <= 4; ++z)
        for (int x = -4; x <= 4; ++x)
            this->chunkManager.requestChunk({center.x+x, 0, center.z+z});
}

// World lifecycle
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

void World::setBlock(const int wx, const int wy, const int wz, const Material id)
{
    const auto [cx, cy, cz] = ChunkPos::fromWorld(wx, wy, wz);
    const auto [x, y, z] = BlockPos::fromWorld(wx, wy, wz);

    Chunk* chunk = this->chunkManager.getChunk(cx, cy, cz);

    if (!chunk || chunk->getState() != ChunkState::READY)
        return;

    chunk->setBlock(x, y, z, id);
    chunk->setDirty(true);

    this->chunkManager.rebuildNeighbors({cx, cy, cz});
}

void World::fill(const glm::ivec3 from, const glm::ivec3 to, const Material id)
{
    for (int z = from.z; z <= to.z; ++z)
        for (int y = from.y; y <= to.y; ++y)
            for (int x = from.x; x <= to.x; ++x)
            {
                this->setBlock(x, y, z, id);
            }
}

// Updates
void World::update(const glm::vec3& cameraPos)
{
    for (auto& chunk : chunkManager.getChunks() | std::views::values) {
        if (chunk.hasPendingChanges())
            chunk.swapBuffers();
    }

    this->chunkManager.updateStreaming(cameraPos);
    this->meshManager.scheduleMeshing(cameraPos);
    this->meshManager.update();
}

void World::render()
{
    this->shader.use();

    for (const auto chunk : this->chunkManager.getRenderableChunks()) {
        const auto& mesh = this->meshManager.getMesh(chunk->getPosition());

        this->shader.setModelMatrix(chunk->getChunkModel());
        mesh.render();
    }
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

const Shader& World::getShader() const
{
    return this->shader;
}
