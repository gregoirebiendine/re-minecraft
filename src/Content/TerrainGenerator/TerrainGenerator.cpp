#include "TerrainGenerator.h"

TerrainGenerator::TerrainGenerator(const BlockRegistry& _blockRegistry, const PrefabRegistry& _prefabRegistry) :
    blockRegistry{_blockRegistry},
    prefabRegistry{_prefabRegistry}
{
    this->noise.SetFrequency(0.015);
}

int TerrainGenerator::getTerrainHeight(const int worldX, const int worldZ) const
{
    const float n = this->noise.GetNoise(static_cast<float>(worldX), static_cast<float>(worldZ));
    return BASE_HEIGHT + static_cast<int>(n * AMPLITUDE);
}

void TerrainGenerator::generate(Chunk& chunk) const
{
    for (int x = 0; x < Chunk::SIZE; x++) {
        for (int z = 0; z < Chunk::SIZE; z++) {
            const auto [cx, cy, cz] = chunk.getPosition();

            const int wx = cx * Chunk::SIZE + x;
            const int wz = cz * Chunk::SIZE + z;
            const int height = this->getTerrainHeight(wx, wz);

            for (int y = 0; y < Chunk::SIZE; y++) {
                const int wy = cy * Chunk::SIZE + y;

                Material mat;
                if (wy < 2)
                    mat = this->blockRegistry.getByName("core:stone");
                else if (wy < height)
                    mat = this->blockRegistry.getByName("core:dirt");
                else if (wy == height)
                    mat = this->blockRegistry.getByName("core:grass");
                else
                    mat = this->blockRegistry.getByName("core:air");

                chunk.setBlockDirect(x, y, z, mat);
            }
        }
    }

    // chunk.finalizeGeneration();
}

void TerrainGenerator::decorate(const Chunk& chunk, NeighborAccess& neighbors) const
{
    this->placePrefab(neighbors, "oak_tree_1", chunk.getPosition());
    neighbors.markDirtyChunks();
}

uint32_t TerrainGenerator::getDecorationSeed(const int chunkX, const int chunkZ) const
{
    return static_cast<uint32_t>(this->seed) ^
           (static_cast<uint32_t>(chunkX) * 73856093u) ^
           (static_cast<uint32_t>(chunkZ) * 19349663u);
}

void TerrainGenerator::placePrefab(NeighborAccess& neighbors, const std::string& prefabName, const ChunkPos& pos) const
{
    const PrefabMeta& prefab = this->prefabRegistry.get(prefabName);
    const int chunkMinY = pos.y * Chunk::SIZE;
    const int chunkMaxY = chunkMinY + Chunk::SIZE - 1;

    const uint32_t localSeed = this->getDecorationSeed(pos.x, pos.z);
    std::mt19937 rng(localSeed);
    std::uniform_int_distribution xDist(2, 13);
    std::uniform_int_distribution zDist(2, 13);
    std::uniform_int_distribution count(0, prefab.density);

    const int maxInstance = count(rng);

    for (int i = 0; i < maxInstance; i++) {
        const int lx = xDist(rng);
        const int lz = zDist(rng);
        const int worldX = pos.x * 16 + lx;
        const int worldZ = pos.z * 16 + lz;
        const int groundY = findGroundLevel(neighbors, worldX, worldZ);

        if (!(groundY >= chunkMinY && groundY <= chunkMaxY))
            continue;

        const Material groundBlock = neighbors.getBlock(worldX, groundY, worldZ);

        if (!this->blockRegistry.isEqual(groundBlock, "core:grass"))
            continue;

        for (const auto& block : prefab.blocks) {
            const int bx = worldX + block.x;
            const int by = groundY + block.y + 1;
            const int bz = worldZ + block.z;

            const Material existing = neighbors.getBlock(bx, by, bz);

            if (BlockData::getBlockId(existing) == 0)
                neighbors.setBlock(bx, by, bz, block.mat);
        }
    }
}

int TerrainGenerator::findGroundLevel(const NeighborAccess& neighbors, const int worldX, const int worldZ)
{
    for (int y = 128; y >= 0; y--) {
        if (const Material mat = neighbors.getBlock(worldX, y, worldZ); BlockData::getBlockId(mat) != 0)
            return y;
    }
    return -1;
}