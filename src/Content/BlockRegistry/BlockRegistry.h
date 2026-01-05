#ifndef RE_MINECRAFT_BLOCKREGISTRY_H
#define RE_MINECRAFT_BLOCKREGISTRY_H

#include <array>
#include <memory>
#include <vector>
#include <unordered_map>
#include <limits>

using BlockAtlasFaces = std::array<uint8_t, 6>;
using Material = uint16_t;

enum MaterialFace : char
{
    FRONT = 0,
    BACK,
    LEFT,
    RIGHT,
    TOP,
    BOTTOM
};

struct BlockMeta
{
    std::string registerNamespace; // core, mod_name, etc
    std::string blockName; // air, dirt, cobble, etc
    bool transparent;
    float hardness;
    BlockAtlasFaces atlasFaces;

    [[nodiscard]] std::string getFullName() const
    {
        return registerNamespace + ":" + blockName;
    }

    [[nodiscard]] std::string getNamespace() const
    {
        return registerNamespace;
    }

    [[nodiscard]] std::string getName() const
    {
        return blockName;
    }
};

class BlockRegistry
{
    std::vector<BlockMeta> blocks;
    std::unordered_map<std::string, Material> nameToMaterialId;

    public:
        BlockRegistry();

        Material registerBlock(const BlockMeta& meta);
        const BlockMeta& get(Material id) const;
        Material getByName(const std::string& name) const;
        bool isEqual(Material id, const std::string& name) const;
};

#endif //RE_MINECRAFT_BLOCKREGISTRY_H