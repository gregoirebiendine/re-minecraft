#include "BlockRegistry.h"

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
        {
            {MaterialFace::NORTH, "dirt"},
            {MaterialFace::SOUTH, "dirt"},
            {MaterialFace::WEST, "dirt"},
            {MaterialFace::EAST, "dirt"},
            {MaterialFace::UP, "dirt"},
            {MaterialFace::DOWN, "dirt"}
        }
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
        {
            {MaterialFace::NORTH, "grass_block_top"},
            {MaterialFace::SOUTH, "grass_block_top"},
            {MaterialFace::WEST, "grass_block_top"},
            {MaterialFace::EAST, "grass_block_top"},
            {MaterialFace::UP, "grass_block_top"},
            {MaterialFace::DOWN, "grass_block_top"}
        }
    });

    this->registerBlock({
        "core",
        "cobble",
        false,
        2.f,
        {
            {MaterialFace::NORTH, "cobble"},
            {MaterialFace::SOUTH, "cobble"},
            {MaterialFace::WEST, "cobble"},
            {MaterialFace::EAST, "cobble"},
            {MaterialFace::UP, "cobble"},
            {MaterialFace::DOWN, "cobble"}
        }
    });

    this->registerBlock({
        "core",
        "stone",
        false,
        1.5f,
        {
            {MaterialFace::NORTH, "stone"},
            {MaterialFace::SOUTH, "stone"},
            {MaterialFace::WEST, "stone"},
            {MaterialFace::EAST, "stone"},
            {MaterialFace::UP, "stone"},
            {MaterialFace::DOWN, "stone"}
        }
    });

    this->registerBlock({
        "core",
        "oak_plank",
        false,
        1.5f,
        {
            {MaterialFace::NORTH, "oak_plank"},
            {MaterialFace::SOUTH, "oak_plank"},
            {MaterialFace::WEST, "oak_plank"},
            {MaterialFace::EAST, "oak_plank"},
            {MaterialFace::UP, "oak_plank"},
            {MaterialFace::DOWN, "oak_plank"}
        }
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
    if (id >= this->blocks.size()) {
        throw std::out_of_range("Invalid Material");
    }

    return this->blocks[id].getFullName() == name;
}
