#ifndef FARFIELD_ITEMREGISTRY_H
#define FARFIELD_ITEMREGISTRY_H

#include <memory>
#include <vector>
#include <unordered_map>
#include <ranges>

#include "Item.h"
#include "TextureRegistry.h"

struct ItemStack
{
    ItemId itemId = 0;
    std::uint8_t stackSize = 0;
    std::uint16_t durability = 0;
};

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

        ItemStack createStack(ItemId id, uint8_t count = 1) const;
        ItemStack createStack(const std::string& identifier, uint8_t count = 1) const;

        std::vector<ItemId> getAll() const;
};

#endif