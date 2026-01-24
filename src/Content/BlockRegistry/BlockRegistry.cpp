#include "BlockRegistry.h"

#include <ranges>

BlockRegistry::BlockRegistry()
{
    this->registerBlock({
        "core",
        "air",
        true,
        0.f,
        {}
    });

    this->registerBlock({
        "core",
        "dirt",
        false,
        1.f,
        uniformBlockFaces("dirt")
    });

    this->registerBlock({
        "core",
        "grass",
        false,
        1.f,
        {
            {MaterialFace::NORTH, "grass_block_side"},
            {MaterialFace::SOUTH, "grass_block_side"},
            {MaterialFace::WEST, "grass_block_side"},
            {MaterialFace::EAST, "grass_block_side"},
            {MaterialFace::UP, "grass_block_top"},
            {MaterialFace::DOWN, "dirt"}
        }
    });

    this->registerBlock({
        "core",
        "moss",
        false,
        1.f,
        uniformBlockFaces("grass_block_top")
    });

    this->registerBlock({
        "core",
        "cobble",
        false,
        2.f,
        uniformBlockFaces("cobble")
    });

    this->registerBlock({
        "core",
        "stone",
        false,
        1.5f,
        uniformBlockFaces("stone")
    });

    this->registerBlock({
        "core",
        "oak_plank",
        false,
        1.5f,
        uniformBlockFaces("oak_plank")
    });

    this->registerBlock({
        "core",
        "oak_log",
        false,
        1.5f,
        {
            {MaterialFace::NORTH, "oak_log"},
            {MaterialFace::SOUTH, "oak_log"},
            {MaterialFace::WEST, "oak_log"},
            {MaterialFace::EAST, "oak_log"},
            {MaterialFace::UP, "oak_log_top"},
            {MaterialFace::DOWN, "oak_log_top"}
        }
    });

    this->registerBlock({
        "core",
        "oak_leaves",
        true,
        0.1f,
        uniformBlockFaces("oak_leaves")
    });

    this->registerBlock({
        "core",
        "coal_block",
        false,
        2.f,
        uniformBlockFaces("coal_block")
    });

    this->registerBlock({
        "core",
        "coal_ore",
        false,
        2.f,
        uniformBlockFaces("coal_ore")
    });

    this->registerBlock({
        "core",
        "iron_block",
        false,
        2.f,
        uniformBlockFaces("iron_block")
    });

    this->registerBlock({
        "core",
        "iron_ore",
        false,
        2.f,
        uniformBlockFaces("iron_ore")
    });

    this->registerBlock({
        "core",
        "gold_block",
        false,
        2.f,
        uniformBlockFaces("gold_block")
    });

    this->registerBlock({
        "core",
        "gold_ore",
        false,
        2.f,
        uniformBlockFaces("gold_ore")
    });

    this->registerBlock({
        "core",
        "redstone_block",
        false,
        2.f,
        uniformBlockFaces("redstone_block")
    });

    this->registerBlock({
        "core",
        "redstone_ore",
        false,
        2.f,
        uniformBlockFaces("redstone_ore")
    });

    this->registerBlock({
        "core",
        "lapis_block",
        false,
        2.f,
        uniformBlockFaces("lapis_block")
    });

    this->registerBlock({
        "core",
        "lapis_ore",
        false,
        2.f,
        uniformBlockFaces("lapis_ore")
    });

    this->registerBlock({
        "core",
        "diamond_block",
        false,
        2.f,
        uniformBlockFaces("diamond_block")
    });

    this->registerBlock({
        "core",
        "diamond_ore",
        false,
        2.f,
        uniformBlockFaces("diamond_ore")
    });

    this->registerBlock({
        "core",
        "emerald_block",
        false,
        2.f,
        uniformBlockFaces("emerald_block")
    });

    this->registerBlock({
        "core",
        "emerald_ore",
        false,
        2.f,
        uniformBlockFaces("emerald_ore")
    });
}

Material BlockRegistry::registerBlock(const BlockMeta& meta)
{
    // Prevent duplication
    auto it = this->nameToMaterialId.find(meta.getFullName());
    if (it != this->nameToMaterialId.end()) {
        return it->second; // already registered
    }

    // Enforce max ID range
    if (this->blocks.size() >= std::numeric_limits<Material>::max()) {
        throw std::runtime_error("BlockRegistry overflow");
    }

    auto id = static_cast<Material>(blocks.size());

    this->blocks.push_back(meta);
    this->nameToMaterialId.emplace(meta.getFullName(), id);

    return id;
}

const BlockMeta& BlockRegistry::get(const Material id) const
{
    if (id >= this->blocks.size())
        throw std::out_of_range("Invalid Material");
    return this->blocks[id];
}

Material BlockRegistry::getByName(const std::string& name) const
{
    if (!this->nameToMaterialId.contains(name))
        return 0;
    return this->nameToMaterialId.at(name);
}

bool BlockRegistry::isEqual(const Material id, const std::string& name) const
{
    if (id >= this->blocks.size())
        throw std::out_of_range("Invalid Material");

    return this->blocks[id].getFullName() == name;
}

std::vector<Material> BlockRegistry::getAll() const
{
    std::vector<Material> materials;

    materials.reserve(this->nameToMaterialId.size());
    for (const auto& id : this->nameToMaterialId | std::views::values)
        materials.push_back(id);
    return materials;
}

// Statics
BlockFaces BlockRegistry::uniformBlockFaces(std::string texture)
{
    return {
        {MaterialFace::NORTH, texture},
        {MaterialFace::SOUTH, texture},
        {MaterialFace::WEST, texture},
        {MaterialFace::EAST, texture},
        {MaterialFace::UP, texture},
        {MaterialFace::DOWN, texture}
    };
}
