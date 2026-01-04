#include "World.h"

World::World()
{
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
    noise.SetFrequency(0.030);
    noise.SetSeed(3120);

    for (int cx = -1; cx <= 1; cx++) {
        for (int cz = -1; cz <= 1; cz++) {
            for (int cy = 0; cy <= 2; cy++) {
                Chunk& chunk = this->getOrCreateChunk(cx, cy, cz);
                generateChunkTerrain(chunk, noise);
            }
        }
    }
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
    this->markNeighborsDirty(pos);
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

bool World::isAir(const int wx, const int wy, const int wz) const
{
    return this->getBlock(wx, wy, wz) == Material::AIR;
}

bool World::chunkExist(const int cx, const int cy, const int cz) const
{
    const ChunkPos pos{cx, cy, cz};
    return this->chunks.contains(pos);
}

void World::setBlock(const int wx, const int wy, const int wz, const Material id)
{
    const auto cp = worldToChunk(wx, wy, wz);
    Chunk& chunk = getOrCreateChunk(cp.x, cp.y, cp.z);

    const int lx = World::mod(wx, Chunk::SIZE);
    const int ly = World::mod(wy, Chunk::SIZE);
    const int lz = World::mod(wz, Chunk::SIZE);

    chunk.setBlock(lx, ly, lz, id);

    if (lx == 0 || lx == Chunk::SIZE - 1 || ly == 0 || ly == Chunk::SIZE - 1 || lz == 0 || lz == Chunk::SIZE - 1)
        this->markNeighborsDirty(cp);
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

void World::markNeighborsDirty(const ChunkPos& cp)
{
    static const glm::ivec3 neighbors[6] = {
        { 1, 0, 0}, {-1, 0, 0},
        { 0, 1, 0}, { 0,-1, 0},
        { 0, 0, 1}, { 0, 0,-1},
    };

    for (auto& n : neighbors) {
        const ChunkPos pos{cp.x + n[0], cp.y + n[1], cp.z + n[2]};

        if (const auto neighbor = this->getChunk(pos.x, pos.y, pos.z))
            neighbor->setDirty(true);
    }
}

int World::getTerrainHeight(const int worldX, const int worldZ, const FastNoiseLite &noise)
{
    const float n = noise.GetNoise(static_cast<float>(worldX), static_cast<float>(worldZ));
    constexpr int baseHeight = 10;
    constexpr int amplitude = 8;

    return baseHeight + static_cast<int>(n * amplitude);
}

void World::generateChunkTerrain(Chunk& chunk, const FastNoiseLite &noise)
{
    for (int x = 0; x < Chunk::SIZE; x++) {
        for (int z = 0; z < Chunk::SIZE; z++) {
            const auto [cx, cy, cz] = chunk.getPosition();

            const int wx = cx * Chunk::SIZE + x;
            const int wz = cz * Chunk::SIZE + z;
            const int height = getTerrainHeight(wx, wz, noise);

            for (int y = 0; y < Chunk::SIZE; y++) {
                int wy = cy * Chunk::SIZE + y;

                if (wy < height)
                    chunk.setBlock(x, y, z, Material::DIRT);
                else if (wy == height)
                    chunk.setBlock(x, y, z, Material::GRASS);
                else
                    chunk.setBlock(x, y, z, Material::AIR);
            }
        }
    }
}


void World::render(const Shader& shaders)
{
    shaders.use();

    for (const auto& chunk : this->chunks | std::views::values)
    {
        ChunkMesh& mesh = meshManager.get(*chunk);

        if (chunk->isDirty())
            mesh.rebuild(*chunk, *this);

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
