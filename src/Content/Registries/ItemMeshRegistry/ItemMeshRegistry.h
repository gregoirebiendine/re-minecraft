#ifndef FARFIELD_ITEMMESHREGISTRY_H
#define FARFIELD_ITEMMESHREGISTRY_H

#include <cstdint>
#include <vector>
#include <unordered_map>

#include "EntityMeshData.h"
#include "ItemRegistry.h"
#include "TextureRegistry.h"

using ItemMeshId = std::uint16_t; // 16bits

class ItemMeshRegistry
{
    std::vector<std::shared_ptr<EntityMeshData>> meshes;
    std::unordered_map<std::string, ItemMeshId> nameToId;

    public:
        explicit ItemMeshRegistry(const TextureRegistry& textureRegistry, const ItemRegistry& itemRegistry);

        ItemMeshId registerItemMesh(const std::string& fullIdentifier, const stbi_uc* texData);

        std::shared_ptr<EntityMeshData> get(ItemMeshId id) const;

        std::shared_ptr<EntityMeshData> get(const std::string &name) const;
        ItemMeshId getIdByName(const std::string& name) const;
};

#endif