#include "TerrainGenerator.h"

TerrainGenerator::TerrainGenerator(const BlockRegistry& _blockRegistry) :
    blockRegistry{_blockRegistry}
{
    this->noise.SetFrequency(0.015);

    this->prefabs.push_back(
        this->loadPrefab("oak_tree_1.json")
    );
}

Prefab TerrainGenerator::loadPrefab(const std::string& name) const
{
    const auto path = fs::current_path().parent_path().string().append("/resources/data/prefabs/").append(name);

    std::ifstream file(path);
    json data = json::parse(file);

    if (data.is_null() || !data.contains("rules") || !data.contains("content") )
        throw std::runtime_error("[Prefab] Loaded file isn't valid : " + name);

    std::vector<PrefabBlockData> blocks;

    for (const auto& content : data["content"])
    {
        if (content["command"].get<std::string>() == "FILL")
        {
            const std::array<int, 3> start = content["start"];
            const std::array<int, 3> stop = content["stop"];
            const Material mat = BlockData::packBlockData(
                this->blockRegistry.getByName(content["block"].get<std::string>()),
                content["rotation"].get<int>()
            );

            for (int y = start[1]; y <= stop[1]; y++)
                blocks.push_back({0, y, 0, mat});
        }
        else if (content["command"].get<std::string>() == "SINGLE")
        {
            const std::array<int, 3> start = content["start"];

            blocks.push_back({
            0, start[1], 0,
            BlockData::packBlockData(
                    blockRegistry.getByName(content["block"].get<std::string>()),
                    content["rotation"].get<int>()
                )
            });
        }
    }

    file.close();

    return {
        data["rules"]["density_per_chunk"].get<int>(),
        blockRegistry.getByName(data["rules"]["block_below"].get<std::string>()),
        blocks
    };
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

    const glm::ivec3 tree1 = { 5, this->getTerrainHeight(5,5), 5};
    const glm::ivec3 tree2 = { 9, this->getTerrainHeight(9,12), 12};

    for (const auto& blockData : this->prefabs[0].blocks)
    {
        const auto& blockBelow = chunk.getBlock(tree1.x, tree1.y - 1, tree1.z);
        const auto& blockBelow2 = chunk.getBlock(tree2.x, tree2.y - 1, tree2.z);

        if (blockBelow == this->blockRegistry.getByName("core:grass"))
        {
            chunk.setBlockDirect(
                tree1.x + blockData.x,
                tree1.y + blockData.y,
                tree1.z + blockData.z,
                blockData.mat
            );
        }

        if (blockBelow2 == this->blockRegistry.getByName("core:grass"))
        {
            chunk.setBlockDirect(
                tree2.x + blockData.x,
                tree2.y + blockData.y,
                tree2.z + blockData.z,
                blockData.mat
            );
        }
    }

    chunk.finalizeGeneration();
}