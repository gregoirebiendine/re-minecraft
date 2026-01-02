#include "World.h"

World::World()
{
    this->getOrCreateChunk(0, 0, 0);
    this->getOrCreateChunk(1, 0, 0);
}

Chunk* World::getChunk(const int cx, const int cy, const int cz)
{
    const ChunkPos pos{cx, cy, cz};

    const auto it = chunks.find(pos);
    return it != chunks.end() ? it->second.get() : nullptr;
}

Chunk& World::getOrCreateChunk(const int cx, const int cy, const int cz)
{
    const ChunkPos pos{cx, cy, cz};

    std::unique_ptr<Chunk>& chunk = this->chunks[pos];

    if (!chunk)
        chunk = std::make_unique<Chunk>(pos);
    return *chunk;
}

Material World::getBlock(const int wx, const int wy, const int wz) const
{
    const ChunkPos cp = worldToChunk(wx, wy, wz);
    const auto it = chunks.find(cp);

    if (it == chunks.end())
        return Material::AIR;

    const int lx = World::mod(wx, Chunk::SIZE);
    const int ly = World::mod(wy, Chunk::SIZE);
    const int lz = World::mod(wz, Chunk::SIZE);

    return it->second->getBlock(lx, ly, lz);
}

void World::setBlock(const int wx, const int wy, const int wz, const Material id)
{
    ChunkPos cp = worldToChunk(wx, wy, wz);
    Chunk& chunk = getOrCreateChunk(cp.x, cp.y, cp.z);

    const int lx = World::mod(wx, Chunk::SIZE);
    const int ly = World::mod(wy, Chunk::SIZE);
    const int lz = World::mod(wz, Chunk::SIZE);

    chunk.setBlock(lx, ly, lz, id);
    // markNeighborsDirty(cp);
}

void World::render(const Shader& shaders)
{
    shaders.use();

    for (const auto& chunk : this->chunks | std::views::values)
    {
        ChunkMesh& mesh = meshManager.get(*chunk);

        if (chunk->isDirty())
            mesh.rebuild(*chunk);

        const glm::mat4 model = chunk->getChunkModel();
        shaders.setUniformMat4("ViewModel", model);

        mesh.render();
    }
}

// Statics
int World::mod(const int a, const int b)
{
    const int r = a % b;
    return r < 0 ? r + b : r;
}

ChunkPos World::worldToChunk(const int wx, const int wy, const int wz)
{
    return {
        World::floorDiv(wx, Chunk::SIZE),
        World::floorDiv(wy, Chunk::SIZE),
        World::floorDiv(wz, Chunk::SIZE)
    };
}

int World::floorDiv(const int a, const int b)
{
    int q = a / b;
    const int r = a % b;

    if (r != 0 && ((r < 0) != (b < 0)))
        --q;
    return q;
}
