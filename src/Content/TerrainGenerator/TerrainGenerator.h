#ifndef RE_MINECRAFT_TERRAINGENERATOR_H
#define RE_MINECRAFT_TERRAINGENERATOR_H

#include <iostream>
#include <random>

#include <FastNoiseLite.h>
#include <glm/glm.hpp>

#include "BlockRegistry.h"
#include "PrefabRegistry.h"
#include "NeighborAccess.h"
#include "Chunk.h"
#include "Utils.h"

class TerrainGenerator
{
    static constexpr int BASE_HEIGHT = 64;
    static constexpr float AMPLITUDE = 8.f;

    const BlockRegistry& blockRegistry;
    const PrefabRegistry& prefabRegistry;

    const int seed = 3120;
    FastNoiseLite noise{seed};

    [[nodiscard]] int getTerrainHeight(int worldX, int worldZ) const;
    [[nodiscard]] static int findGroundLevel(const NeighborAccess& neighbors, int worldX, int worldZ);
    [[nodiscard]] uint32_t getDecorationSeed(int chunkX, int chunkZ) const;

    void placePrefab(NeighborAccess& neighbors, const std::string& prefabName, const ChunkPos& pos) const;

    public:
        explicit TerrainGenerator(const BlockRegistry& _blockRegistry, const PrefabRegistry& _prefabRegistry);

        void generate(Chunk& chunk) const;
        void decorate(const Chunk& chunk, NeighborAccess& neighbors) const;
};

#endif //RE_MINECRAFT_TERRAINGENERATOR_H