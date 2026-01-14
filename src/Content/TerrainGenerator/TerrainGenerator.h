#ifndef RE_MINECRAFT_TERRAINGENERATOR_H
#define RE_MINECRAFT_TERRAINGENERATOR_H

#include <FastNoiseLite.h>
#include <glm/glm.hpp>

#include "BlockRegistry.h"
#include "Chunk.h"

class TerrainGenerator
{
    const BlockRegistry& blockRegistry;
    FastNoiseLite noise;
    int baseHeight;
    int amplitude;

    public:
        TerrainGenerator(int _baseHeight, int _amplitude, const BlockRegistry& _blockRegistry);

        [[nodiscard]] int getTerrainHeight(int worldX, int worldZ) const;
        void generateChunkTerrain(Chunk& chunk) const;
};


#endif //RE_MINECRAFT_TERRAINGENERATOR_H