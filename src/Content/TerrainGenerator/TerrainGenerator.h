#ifndef RE_MINECRAFT_TERRAINGENERATOR_H
#define RE_MINECRAFT_TERRAINGENERATOR_H

#include <iostream>
#include <random>

#include <FastNoiseLite.h>
#include <glm/glm.hpp>

#include "BlockRegistry.h"
#include "PrefabRegistry.h"
#include "Chunk.h"
#include "Utils.h"

class TerrainGenerator
{
    const BlockRegistry& blockRegistry;
    const PrefabRegistry& prefabRegistry;

    FastNoiseLite noise{3120};
    const int baseHeight = 64;
    const int amplitude = 8;

    public:
        explicit TerrainGenerator(const BlockRegistry& _blockRegistry, const PrefabRegistry& _prefabRegistry);

        [[nodiscard]] int getTerrainHeight(int worldX, int worldZ) const;
        void generate(Chunk& chunk) const;
};

#endif //RE_MINECRAFT_TERRAINGENERATOR_H