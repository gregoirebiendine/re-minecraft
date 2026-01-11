#include "BlockRegistry.h"

BlockRegistry::BlockRegistry()
{
    this->registerBlock({
        "core",
        "air",
        true,
        0.f,
        {
            {MaterialFace::NORTH, 0},
            {MaterialFace::SOUTH, 0},
            {MaterialFace::WEST, 0},
            {MaterialFace::EAST, 0},
            {MaterialFace::UP, 0},
            {MaterialFace::DOWN, 0},
        }
    });

    this->registerBlock({
        "core",
        "dirt",
        false,
        1.f,
        {
            {MaterialFace::NORTH, 0},
            {MaterialFace::SOUTH, 0},
            {MaterialFace::WEST, 0},
            {MaterialFace::EAST, 0},
            {MaterialFace::UP, 0},
            {MaterialFace::DOWN, 0}
        }
    });

    this->registerBlock({
        "core",
        "grass",
        false,
        1.f,
        {
            {MaterialFace::NORTH, 2},
            {MaterialFace::SOUTH, 5},
            {MaterialFace::WEST, 3},
            {MaterialFace::EAST, 4},
            {MaterialFace::UP, 1},
            {MaterialFace::DOWN, 0}
        }
    });

    this->registerBlock({
        "core",
        "moss",
        false,
        1.f,
        {
            {MaterialFace::NORTH, 2},
            {MaterialFace::SOUTH, 2},
            {MaterialFace::WEST, 2},
            {MaterialFace::EAST, 2},
            {MaterialFace::UP, 2},
            {MaterialFace::DOWN, 2}
        }
    });

    this->registerBlock({
        "core",
        "cobble",
        false,
        2.f,
        {
            {MaterialFace::NORTH, 3},
            {MaterialFace::SOUTH, 3},
            {MaterialFace::WEST, 3},
            {MaterialFace::EAST, 3},
            {MaterialFace::UP, 3},
            {MaterialFace::DOWN, 3}
        }
    });

    this->registerBlock({
        "core",
        "oak_plank",
        false,
        1.5f,
        {
            {MaterialFace::NORTH, 4},
            {MaterialFace::SOUTH, 4},
            {MaterialFace::WEST, 4},
            {MaterialFace::EAST, 4},
            {MaterialFace::UP, 4},
            {MaterialFace::DOWN, 4}
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
    if (id >= this->blocks.size()) {
        throw std::out_of_range("Invalid Material");
    }

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
