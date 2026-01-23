#ifndef RE_MINECRAFT_BLOCKREGISTRY_H
#define RE_MINECRAFT_BLOCKREGISTRY_H

#include <memory>
#include <vector>
#include <unordered_map>
#include <map>
#include <limits>

#include "Material.h"
#include "TextureRegistry.h"

using BlockFaces = std::map<MaterialFace, std::string>;

struct BlockMeta
{
    std::string registerNamespace; // "core", "mod_name", etc
    std::string blockName; // "air", "dirt", "cobble", etc
    bool transparent;
    float hardness;
    BlockFaces blockFaces;

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

    [[nodiscard]] std::string getFaceTexture(const MaterialFace& face) const
    {
        const auto it = blockFaces.find(face);

        if (it == blockFaces.end())
            return TextureRegistry::MISSING;
        return it->second;
    }
};

class BlockRegistry
{
    std::vector<BlockMeta> blocks;
    std::unordered_map<std::string, Material> nameToMaterialId;

    static BlockFaces uniformBlockFaces(std::string texture);

    public:
        BlockRegistry();

        Material registerBlock(const BlockMeta& meta);
        const BlockMeta& get(Material id) const;
        Material getByName(const std::string& name) const;
        bool isEqual(Material id, const std::string& name) const;
};

#endif //RE_MINECRAFT_BLOCKREGISTRY_H