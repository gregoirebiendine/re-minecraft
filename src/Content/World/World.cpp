#include "World.h"

#include <utility>

World::World(BlockRegistry  _blockRegistry, const TextureRegistry& _textureRegistry) :
    blockRegistry(std::move(_blockRegistry)),
    textureRegistry(_textureRegistry)
{
    this->noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
    this->noise.SetFrequency(0.030);
    this->noise.SetSeed(3120);

    for (int cx = -1; cx <= 1; cx++) {
        for (int cz = -1; cz <= 1; cz++) {
            for (int cy = 0; cy <= 1; cy++) {
                Chunk& chunk = this->getOrCreateChunk(cx, cy, cz);
                this->generateChunkTerrain(chunk);
            }
        }
    }
}

// World lifecycle
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
    {
        chunk = std::make_unique<Chunk>(pos);
        this->markNeighborsDirty(pos);
    }
    return *chunk;
}

Material World::getBlock(const int wx, const int wy, const int wz) const
{
    const ChunkPos cp = blockToChunk(wx, wy, wz);
    const auto it = chunks.find(cp);

    if (it == chunks.end())
        return this->blockRegistry.getByName("core:air");

    const auto [x, y, z] = blockToLocal(wx, wy, wz);
    return it->second->getBlock(x, y, z);
}

bool World::isAir(const int wx, const int wy, const int wz) const
{
    return this->blockRegistry.isEqual(this->getBlock(wx, wy, wz), "core:air");
}

bool World::chunkExist(const int cx, const int cy, const int cz) const
{
    const ChunkPos pos{cx, cy, cz};
    return this->chunks.contains(pos);
}

void World::setBlock(const int wx, const int wy, const int wz, const Material id)
{
    const auto cp = blockToChunk(wx, wy, wz);
    Chunk& chunk = getOrCreateChunk(cp.x, cp.y, cp.z);

    const auto [x, y, z] = blockToLocal(wx, wy, wz);
    chunk.setBlock(x, y, z, id);

    if (x == 0 || x == Chunk::SIZE - 1 || y == 0 || y == Chunk::SIZE - 1 || z == 0 || z == Chunk::SIZE - 1)
        this->markNeighborsDirty(cp, BlockPos({x,y,z}));
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

void World::markNeighborsDirty(const ChunkPos& cp, const std::optional<BlockPos>& bp)
{
    std::vector<glm::ivec3> neighbors;

    if (bp.has_value())
    {
        const auto v = bp.value();
        if (v.x == 0)                  neighbors.emplace_back(-1, 0, 0);
        if (v.x == Chunk::SIZE - 1)    neighbors.emplace_back(1, 0, 0);
        if (v.y == 0)                  neighbors.emplace_back(0, -1, 0);
        if (v.y == Chunk::SIZE - 1)    neighbors.emplace_back(0, 1, 0);
        if (v.z == 0)                  neighbors.emplace_back(0, 0, -1);
        if (v.z == Chunk::SIZE - 1)    neighbors.emplace_back(0, 0, 1);
    } else
    {
        neighbors = {
            { 1, 0, 0}, {-1, 0, 0},
            { 0, 1, 0}, { 0,-1, 0},
            { 0, 0, 1}, { 0, 0,-1},
        };
    }


    for (auto& n : neighbors) {
        const ChunkPos pos{cp.x + n[0], cp.y + n[1], cp.z + n[2]};

        if (const auto chunk = this->getChunk(pos.x, pos.y, pos.z))
            chunk->setDirty(true);
    }
}


// Terrain
int World::getTerrainHeight(const int worldX, const int worldZ) const
{
    const float n = this->noise.GetNoise(static_cast<float>(worldX), static_cast<float>(worldZ));
    constexpr int baseHeight = 10;
    constexpr int amplitude = 8;

    return baseHeight + static_cast<int>(n * amplitude);
}

void World::generateChunkTerrain(Chunk& chunk) const
{
    for (int x = 0; x < Chunk::SIZE; x++) {
        for (int z = 0; z < Chunk::SIZE; z++) {
            const auto [cx, cy, cz] = chunk.getPosition();

            const int wx = cx * Chunk::SIZE + x;
            const int wz = cz * Chunk::SIZE + z;
            const int height = this->getTerrainHeight(wx, wz);

            for (int y = 0; y < Chunk::SIZE; y++) {
                int wy = cy * Chunk::SIZE + y;

                if (wy < 2)
                    chunk.setBlock(x, y, z, this->blockRegistry.getByName("core:stone"));
                else if (wy < height)
                    chunk.setBlock(x, y, z, this->blockRegistry.getByName("core:dirt"));
                else if (wy == height)
                    chunk.setBlock(x, y, z, this->blockRegistry.getByName("core:grass"));
                else
                    chunk.setBlock(x, y, z, this->blockRegistry.getByName("core:air"));
            }
        }
    }
}


// Updates
void World::update()
{
    for (const auto& chunk : this->chunks | std::views::values)
    {
        ChunkMesh& mesh = meshManager.get(*chunk);

        if (chunk->isDirty())
            mesh.rebuild(*chunk, *this);
    }
}

void World::render(const Shader& worldShader)
{
    worldShader.use();

    for (const auto& chunk : this->chunks | std::views::values)
    {
        ChunkMesh& mesh = meshManager.get(*chunk);

        const glm::mat4 model = chunk->getChunkModel();
        worldShader.setUniformMat4("ModelMatrix", model);

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


// Statics
ChunkPos World::blockToChunk(const int wx, const int wy, const int wz)
{
    return {wx >> 4, wy >> 4, wz >> 4};
}

BlockPos World::blockToLocal(const int wx, const int wy, const int wz)
{
    return {wx & 15, wy & 15, wz & 15};
}
