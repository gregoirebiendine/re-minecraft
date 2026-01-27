#ifndef RE_MINECRAFT_TERRAINGENERATOR_H
#define RE_MINECRAFT_TERRAINGENERATOR_H

#include <iostream>

#include <FastNoiseLite.h>
#include <glm/glm.hpp>
#include <json.hpp>

#include "BlockRegistry.h"
#include "Chunk.h"

using json = nlohmann::json;

struct PrefabBlockData
{
    int x, y, z;
    Material mat;
};

struct Prefab
{
    int density;
    BlockId blockBelow;
    std::vector<PrefabBlockData> blocks;
};

class TerrainGenerator
{
    const BlockRegistry& blockRegistry;

    FastNoiseLite noise{3120};
    const int baseHeight = 64;
    const int amplitude = 8;

    std::vector<Prefab> prefabs;

    [[nodiscard]] Prefab loadPrefab(const std::string& name) const;

    public:
        explicit TerrainGenerator(const BlockRegistry& _blockRegistry);

        [[nodiscard]] int getTerrainHeight(int worldX, int worldZ) const;
        void generate(Chunk& chunk) const;
};

#endif //RE_MINECRAFT_TERRAINGENERATOR_H