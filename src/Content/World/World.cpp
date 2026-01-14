#include "World.h"

World::World(BlockRegistry _blockRegistry, const TextureRegistry& _textureRegistry) :
    blockRegistry(std::move(_blockRegistry)),
    textureRegistry(_textureRegistry),
    terrainGenerator(10, 8, blockRegistry)
{
    this->chunkManager = std::make_unique<ChunkManager>();

    if (!this->chunkManager)
        throw std::runtime_error("ChunkManager failed to load");

    for (int cx = -1; cx <= 1; cx++)
    {
        for (int cz = -1; cz <= 1; cz++)
        {
            for (int cy = 0; cy <= 1; cy++)
            {
                Chunk& chunk = this->chunkManager->getOrCreateChunk(cx, cy, cz);
                this->terrainGenerator.generateChunkTerrain(chunk);
            }
        }
    }
}

// World lifecycle
Material World::getBlock(const int wx, const int wy, const int wz) const
{
    const auto [cx, cy, cz] = ChunkPos::fromWorld(wx, wy, wz);
    const Chunk* chunk = this->chunkManager->getChunk(cx, cy, cz);

    if (!chunk)
        return this->blockRegistry.getByName("core:air");

    const auto [x, y, z] = BlockPos::fromWorld(wx, wy, wz);
    return chunk->getBlock(x, y, z);
}

bool World::isAir(const int wx, const int wy, const int wz) const
{
    return this->blockRegistry.isEqual(this->getBlock(wx, wy, wz), "core:air");
}

void World::setBlock(const int wx, const int wy, const int wz, const Material id) const
{
    const auto cp = ChunkPos::fromWorld(wx, wy, wz);
    Chunk& chunk = this->chunkManager->getOrCreateChunk(cp.x, cp.y, cp.z);

    const auto [x, y, z] = BlockPos::fromWorld(wx, wy, wz);
    chunk.setBlock(x, y, z, id);

    if (x == 0 || x == Chunk::SIZE - 1 || y == 0 || y == Chunk::SIZE - 1 || z == 0 || z == Chunk::SIZE - 1)
        this->chunkManager->markNeighborsDirty(cp, BlockPos({x,y,z}));
}

void World::fill(const glm::ivec3 from, const glm::ivec3 to, const Material id) const {
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
    this->chunkManager->update(this->terrainGenerator, cameraPos);

    for (const auto& chunk : this->chunkManager->getChunks() | std::views::values)
    {
        if (chunk->isDirty())
            this->meshManager.get(*chunk).rebuild(*chunk, *this);
    }
}

void World::render(const Shader& worldShader)
{
    worldShader.use();

    for (const auto& chunk : this->chunkManager->getChunks() | std::views::values)
    {
        worldShader.setUniformMat4("ModelMatrix", chunk->getChunkModel());
        meshManager.get(*chunk).render();
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