#include "TerrainGenerator.h"

TerrainGenerator::TerrainGenerator(const int _baseHeight, const int _amplitude, const BlockRegistry& _blockRegistry) :
    blockRegistry(_blockRegistry),
    baseHeight(_baseHeight),
    amplitude(_amplitude)
{
    this->noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
    this->noise.SetFrequency(0.020);
    this->noise.SetSeed(3120);
}

int TerrainGenerator::getTerrainHeight(const int worldX, const int worldZ) const
{
    const float n = this->noise.GetNoise(static_cast<float>(worldX), static_cast<float>(worldZ));
    return baseHeight + static_cast<int>(n * static_cast<float>(amplitude));
}

void TerrainGenerator::generateChunkTerrain(Chunk& chunk) const
{
    for (int x = 0; x < Chunk::SIZE; x++) {
        for (int z = 0; z < Chunk::SIZE; z++) {
            const auto [cx, cy, cz] = chunk.getPosition();

            const int wx = cx * Chunk::SIZE + x;
            const int wz = cz * Chunk::SIZE + z;
            const int height = this->getTerrainHeight(wx, wz);

            for (int y = 0; y < Chunk::SIZE; y++) {
                const int wy = cy * Chunk::SIZE + y;

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
