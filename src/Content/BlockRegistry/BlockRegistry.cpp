#include "BlockRegistry.h"

BlockRegistry::BlockRegistry()
{
    this->registerBlock({
        "core",
        "air",
        true,
        false,
        0.f,
        {}
    });

    this->registerBlock({
        "core",
        "dirt",
        false,
        false,
        1.f,
        uniformBlockFaces("dirt")
    });

    this->registerBlock({
        "core",
        "grass",
        false,
        false,
        1.f,
        {
            {NORTH, "grass_block_side"},
            {SOUTH, "grass_block_side"},
            {WEST, "grass_block_side"},
            {EAST, "grass_block_side"},
            {UP, "grass_block_top"},
            {DOWN, "dirt"}
        }
    });

    this->registerBlock({
        "core",
        "moss",
        false,
        false,
        1.f,
        uniformBlockFaces("grass_block_top")
    });

    this->registerBlock({
        "core",
        "cobble",
        false,
        false,
        2.f,
        uniformBlockFaces("cobble")
    });

    this->registerBlock({
        "core",
        "stone",
        false,
        false,
        1.5f,
        uniformBlockFaces("stone")
    });

    this->registerBlock({
        "core",
        "oak_plank",
        false,
        true,
        1.5f,
        uniformBlockFaces("oak_plank")
    });

    this->registerBlock({
        "core",
        "oak_log",
        false,
        true,
        1.5f,
        {
            {NORTH, "oak_log"},
            {SOUTH, "oak_log"},
            {WEST, "oak_log"},
            {EAST, "oak_log"},
            {UP, "oak_log_top"},
            {DOWN, "oak_log_top"}
        }
    });

    this->registerBlock({
        "core",
        "oak_leaves",
        true,
        false,
        0.1f,
        uniformBlockFaces("oak_leaves")
    });

    this->registerBlock({
        "core",
        "coal_block",
        false,
        false,
        2.f,
        uniformBlockFaces("coal_block")
    });

    this->registerBlock({
        "core",
        "coal_ore",
        false,
        false,
        2.f,
        uniformBlockFaces("coal_ore")
    });

    this->registerBlock({
        "core",
        "iron_block",
        false,
        false,
        2.f,
        uniformBlockFaces("iron_block")
    });

    this->registerBlock({
        "core",
        "iron_ore",
        false,
        false,
        2.f,
        uniformBlockFaces("iron_ore")
    });

    this->registerBlock({
        "core",
        "gold_block",
        false,
        false,
        2.f,
        uniformBlockFaces("gold_block")
    });

    this->registerBlock({
        "core",
        "gold_ore",
        false,
        false,
        2.f,
        uniformBlockFaces("gold_ore")
    });

    this->registerBlock({
        "core",
        "redstone_block",
        false,
        false,
        2.f,
        uniformBlockFaces("redstone_block")
    });

    this->registerBlock({
        "core",
        "redstone_ore",
        false,
        false,
        2.f,
        uniformBlockFaces("redstone_ore")
    });

    this->registerBlock({
        "core",
        "lapis_block",
        false,
        false,
        2.f,
        uniformBlockFaces("lapis_block")
    });

    this->registerBlock({
        "core",
        "lapis_ore",
        false,
        false,
        2.f,
        uniformBlockFaces("lapis_ore")
    });

    this->registerBlock({
        "core",
        "diamond_block",
        false,
        false,
        2.f,
        uniformBlockFaces("diamond_block")
    });

    this->registerBlock({
        "core",
        "diamond_ore",
        false,
        false,
        2.f,
        uniformBlockFaces("diamond_ore")
    });

    this->registerBlock({
        "core",
        "emerald_block",
        false,
        false,
        2.f,
        uniformBlockFaces("emerald_block")
    });

    this->registerBlock({
        "core",
        "emerald_ore",
        false,
        false,
        2.f,
        uniformBlockFaces("emerald_ore")
    });
}

BlockId BlockRegistry::registerBlock(const BlockMeta& meta)
{
    // Prevent duplication
    auto it = this->nameToBlockId.find(meta.getFullName());
    if (it != this->nameToBlockId.end()) {
        return it->second; // already registered
    }

    // Enforce max ID range
    if (this->blocks.size() >= std::numeric_limits<BlockId>::max())
        throw std::runtime_error("BlockRegistry overflow. Farfield has reached its maximum block storage.");

    auto id = static_cast<BlockId>(blocks.size());

    this->blocks.push_back(meta);
    this->nameToBlockId.emplace(meta.getFullName(), id);

    return id;
}

const BlockMeta& BlockRegistry::get(const BlockId id) const
{
    if (id >= this->blocks.size())
        throw std::out_of_range("Out of range BlockID. This means this block id is not registered in the Engine");
    return this->blocks[id];
}

BlockId BlockRegistry::getByName(const std::string& name) const
{
    if (!this->nameToBlockId.contains(name))
        return 0;
    return this->nameToBlockId.at(name);
}

bool BlockRegistry::isEqual(const BlockId id, const std::string& name) const
{
    if (id >= this->blocks.size())
        throw std::out_of_range("Out of range BlockID. This means this block id is not registered in the Engine");

    return this->blocks[id].getFullName() == name;
}

std::vector<BlockId> BlockRegistry::getAll() const
{
    std::vector<BlockId> allBlocks;

    allBlocks.reserve(this->nameToBlockId.size());
    for (const auto& id : this->nameToBlockId | std::views::values)
        allBlocks.push_back(id);
    return allBlocks;
}

// Statics
BlockFaces BlockRegistry::uniformBlockFaces(std::string texture)
{
    return {
        {NORTH, texture},
        {SOUTH, texture},
        {WEST, texture},
        {EAST, texture},
        {UP, texture},
        {DOWN, texture}
    };
}
