#include "ItemRegistry.h"

ItemRegistry::ItemRegistry(const TextureRegistry& _textureRegistry) :
    textureRegistry(_textureRegistry)
{
    const auto& itemTex = this->textureRegistry.getByName("iron_ingot");

    this->registerItem(
        std::make_unique<Item>(NamespaceIdentifier{"iron_ingot"}, "Iron Ingot", "", itemTex, 64)
    );
}

ItemId ItemRegistry::registerItem(std::unique_ptr<Item> _item)
{
    const auto& fullIdentifier = _item->getIdentifier().getFullIdentifier();

    // Prevent duplication
    auto it = this->nameToId.find(fullIdentifier);
    if (it != this->nameToId.end())
        return it->second;

    // Enforce max ID range
    if (this->items.size() >= std::numeric_limits<ItemId>::max())
        throw std::runtime_error("[ItemRegistry::registerItem] ID overflow, maximum Item storage reached");

    auto id = static_cast<ItemId>(this->items.size());

    this->items.emplace_back(std::move(_item));
    this->nameToId.emplace(fullIdentifier, id);

    return id;
}

const Item& ItemRegistry::get(const ItemId id) const
{
    if (id >= this->items.size())
        throw std::out_of_range("[ItemRegistry::get] Out of range ID : " + std::to_string(id));
    return *this->items[id];
}

const Item &ItemRegistry::get(const std::string &name) const
{
    const auto& id = this->getIdByName(name);

    if (id >= this->items.size())
        throw std::out_of_range("[ItemRegistry::get] Out of range ID : " + std::to_string(id));
    return *this->items[id];
}

ItemId ItemRegistry::getIdByName(const std::string& name) const
{
    if (!this->nameToId.contains(name))
        return 0;
    return this->nameToId.at(name);
}


std::vector<ItemId> ItemRegistry::getAll() const
{
    std::vector<ItemId> all;

    all.reserve(this->nameToId.size());
    for (const auto& id : this->nameToId | std::views::values)
        all.push_back(id);
    return all;
}
