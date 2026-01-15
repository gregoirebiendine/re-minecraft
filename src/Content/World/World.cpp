#include "World.h"

World::World(BlockRegistry _blockRegistry, const TextureRegistry& _textureRegistry) :
    blockRegistry(std::move(_blockRegistry)),
    textureRegistry(_textureRegistry),
    meshManager(*this)
{
    this->chunkManager = std::make_unique<ChunkManager>();

    if (!this->chunkManager)
        throw std::runtime_error("ChunkManager failed to load");

    // Create spawn Chunks
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
    for (const auto&[pos, chunk] : this->chunkManager->getChunks()) {
        if (chunk->getState() == ChunkState::READY) {
            const float distance = glm::distance(
                cameraPos,
                glm::vec3(pos.x, pos.y, pos.z)
            );

            meshManager.requestRebuild(*chunk, distance);
        }
    }

    meshManager.update(cameraPos);
}

void World::render(const Shader& worldShader)
{
    // worldShader.use();
    //
    // for (const auto& chunk : this->chunkManager->getChunks() | std::views::values)
    // {
    //     worldShader.setUniformMat4("ModelMatrix", chunk->getChunkModel());
    //     meshManager.get(*chunk).render();
    // }
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

const std::unique_ptr<ChunkManager> &World::getChunkManager() const
{
    return this->chunkManager;
}
