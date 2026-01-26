#ifndef RE_MINECRAFT_BLOCKREGISTRY_H
#define RE_MINECRAFT_BLOCKREGISTRY_H

#include <memory>
#include <vector>
#include <unordered_map>
#include <map>
#include <ranges>
#include <limits>

#include "Material.h"
#include "TextureRegistry.h"

using BlockFaces = std::map<MaterialFace, std::string>;

enum class RotationType : uint8_t {
    NONE,            // No rotation (dirt, stone, etc.)
    HORIZONTAL,      // Rotate on Y-axis only (furnace, chest)
    AXIS             // Rotate based on placement face (logs, pillars)
};

struct BlockMeta
{
    std::string registerNamespace; // "core", "mod_name", etc
    std::string blockName; // "air", "dirt", "cobble", etc
    bool transparent;
    float hardness;
    RotationType rotation;
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
    std::unordered_map<std::string, BlockId> nameToBlockId;

    static BlockFaces uniformBlockFaces(std::string texture);

    public:
        BlockRegistry();

        BlockId registerBlock(const BlockMeta& meta);
        const BlockMeta& get(BlockId id) const;
        BlockId getByName(const std::string& name) const;
        bool isEqual(BlockId id, const std::string& name) const;

        std::vector<BlockId> getAll() const;
};

#endif //RE_MINECRAFT_BLOCKREGISTRY_H