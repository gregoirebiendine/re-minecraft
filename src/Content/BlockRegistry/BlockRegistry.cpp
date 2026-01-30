#include "BlockRegistry.h"

BlockRegistry::BlockRegistry()
{
    this->registerBlock({"core","air",true,0.f,RotationType::NONE,{}});
    this->registerBlocksFromFile("core");
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

void BlockRegistry::registerBlocksFromFile(const std::string& registerNamespace)
{
    const auto blocksFile = fs::current_path().parent_path().string().append("/resources/data/blocks/blocks.json");

    std::ifstream file(blocksFile);
    json data = json::parse(file);

    if (data.is_null() || !data.contains("blocks") || data["blocks"].empty())
        throw std::runtime_error("[BlockRegistry] Loaded file isn't valid : " + blocksFile);

    for (const auto& block : data["blocks"]) {
        if (!block.contains("textures") || block["textures"].empty())
            throw std::runtime_error("[BlockRegistry] Block definition requires an array of textures : " + block["name"].get<std::string>());

        const std::vector<std::string> extractedTextures = block["textures"];
        BlockFaces blockFaces{};

        if (extractedTextures.size() == 1)
            blockFaces = uniformBlockFaces(extractedTextures[0]);
        else if (extractedTextures.size() == 6) {
            blockFaces = {
                {NORTH, extractedTextures[0]},
                {SOUTH, extractedTextures[1]},
                {WEST, extractedTextures[2]},
                {EAST, extractedTextures[3]},
                {UP, extractedTextures[4]},
                {DOWN, extractedTextures[5]},
            };
        }

        this->registerBlock({
            registerNamespace,
            block["name"].get<std::string>(),
            block["transparent"].get<bool>(),
            block["hardness"].get<float>(),
            block["rotation"].get<RotationType>(),
            blockFaces
        });
    }

    file.close();
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
