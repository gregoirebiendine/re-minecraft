#ifndef RE_MINECRAFT_TERRAINGENERATOR_H
#define RE_MINECRAFT_TERRAINGENERATOR_H

#include <FastNoiseLite.h>
#include <glm/glm.hpp>

#include "BlockRegistry.h"
#include "Chunk.h"

class TerrainGenerator
{
    public:
        static inline FastNoiseLite noise{3120};
        static inline int baseHeight = 10;
        static inline int amplitude = 8;

        static void init();
        [[nodiscard]] static int getTerrainHeight(int worldX, int worldZ);
        static void generate(Chunk& chunk, const BlockRegistry& blockRegistry);
};

#endif //RE_MINECRAFT_TERRAINGENERATOR_H