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
    return this->baseHeight + static_cast<int>(n * static_cast<float>(this->amplitude));
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

    // Random x, z
    const int x = Maths::randomInt(5, 10);
    const int z = Maths::randomInt(5, 10);

    // Get height from noise
    const auto [cx, cy, cz] = chunk.getPosition();
    const int wx = cx * Chunk::SIZE + x;
    const int wz = cz * Chunk::SIZE + z;
    const int y = this->getTerrainHeight(wx, wz);

    // Generate a tree
    const auto& tree = this->prefabRegistry.get(this->prefabRegistry.getByName("oak_tree_1"));

    if (BlockData::getBlockId(chunk.getBlock(x, y, z)) == tree.blockBelow)
    {
        for (const auto& [px, py, pz, mat] : tree.blocks)
            chunk.setBlockDirect(
                x + px,
                y + py + 1,
                z + pz,
                mat
            );
    }

    chunk.finalizeGeneration();
}