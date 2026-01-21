#include "TerrainGenerator.h"

void TerrainGenerator::init()
{
    noise.SetFrequency(0.020);
}

int TerrainGenerator::getTerrainHeight(const int worldX, const int worldZ)
{
    const float n = noise.GetNoise(static_cast<float>(worldX), static_cast<float>(worldZ));
    return baseHeight + static_cast<int>(n * static_cast<float>(amplitude));
}

void TerrainGenerator::generate(Chunk& chunk, const BlockRegistry& blockRegistry)
{
    for (int x = 0; x < Chunk::SIZE; x++) {
        for (int z = 0; z < Chunk::SIZE; z++) {
            const auto [cx, cy, cz] = chunk.getPosition();

            const int wx = cx * Chunk::SIZE + x;
            const int wz = cz * Chunk::SIZE + z;
            const int height = getTerrainHeight(wx, wz);

            for (int y = 0; y < Chunk::SIZE; y++) {
                const int wy = cy * Chunk::SIZE + y;

                Material mat;
                if (wy < 2)
                    mat = blockRegistry.getByName("core:stone");
                else if (wy < height)
                    mat = blockRegistry.getByName("core:dirt");
                else if (wy == height)
                    mat = blockRegistry.getByName("core:grass");
                else
                    mat = blockRegistry.getByName("core:air");

                chunk.setBlockDirect(x, y, z, mat);  // Direct write, no buffering
            }
        }
    }

    chunk.finalizeGeneration();
}