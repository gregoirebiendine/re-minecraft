#ifndef RE_MINECRAFT_TERRAINGENERATOR_H
#define RE_MINECRAFT_TERRAINGENERATOR_H

#include <FastNoiseLite.h>
#include <glm/glm.hpp>

#include "BlockRegistry.h"
#include "Chunk.h"

class TerrainGenerator
{
    static  FastNoiseLite noise;
    static int baseHeight;
    static int amplitude;

    public:
        static void init();
        [[nodiscard]] static int getTerrainHeight(int worldX, int worldZ);
        static void generate(Chunk& chunk, const BlockRegistry& blockRegistry);
};


#endif //RE_MINECRAFT_TERRAINGENERATOR_H