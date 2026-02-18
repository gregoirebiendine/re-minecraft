#ifndef FARFIELD_ITEMREGISTRY_H
#define FARFIELD_ITEMREGISTRY_H

#include <memory>
#include <vector>
#include <unordered_map>
#include <ranges>

#include "Item.h"
#include "TextureRegistry.h"

class ItemRegistry
{
    const TextureRegistry& textureRegistry;

    std::vector<std::unique_ptr<Item>> items;
    std::unordered_map<std::string, ItemId> nameToId;

    public:
        explicit ItemRegistry(const TextureRegistry& _textureRegistry);

        ItemId registerItem(std::unique_ptr<Item> _item);

        const Item& get(ItemId id) const;
        const Item& get(const std::string& name) const;
        ItemId getIdByName(const std::string& name) const;

        std::vector<ItemId> getAll() const;
};

#endif