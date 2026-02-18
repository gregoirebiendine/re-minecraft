#ifndef FARFIELD_ITEM_H
#define FARFIELD_ITEM_H

#include <cstdint>
#include <string>
#include <utility>

#include "TextureRegistry.h"

using ItemId = std::uint16_t; // 16bits

struct NamespaceIdentifier
{
    private:
        std::string identifier; // Namespace -> "core", "mod1", ...
        std::string name;       // Object name -> "iron_ingot", "iron_block", ...

    public:
        NamespaceIdentifier(std::string _identifier, std::string _name) :
            identifier(std::move(_identifier)),
            name(std::move(_name))
        {}

        explicit NamespaceIdentifier(std::string _name) :
            identifier("core"),
            name(std::move(_name))
        {}

        [[nodiscard]] std::string getNamespace() const
        {
            return this->identifier;
        }

        [[nodiscard]] std::string getName() const
        {
            return this->name;
        }

        [[nodiscard]] std::string getFullIdentifier() const
        {
            return this->identifier + ":" + this->name;
        }
};

class Item
{
    protected:
        NamespaceIdentifier identifier; // "core:iron_ingot"

        std::string displayName;
        std::string lore;

        std::uint16_t maxStackSize;

        TextureId textureId = 0;

    public:
        Item(NamespaceIdentifier _identifier, std::string _displayName, std::string _lore, const TextureId texId, const std::uint16_t _maxStackSize) :
            identifier(std::move(_identifier)),
            displayName(std::move(_displayName)),
            lore(std::move(_lore)),
            textureId(texId),
            maxStackSize(_maxStackSize)
        {}

        virtual ~Item() = default;

        virtual void use() {}
        virtual void useOn() {}

        [[nodiscard]] TextureId getIcon() const
        {
            return this->textureId;
        }

        [[nodiscard]] const NamespaceIdentifier& getIdentifier()
        {
            return this->identifier;
        }

        [[nodiscard]] std::string getDisplayName() const
        {
            return this->displayName;
        }

        [[nodiscard]] std::string getLore() const
        {
            return this->lore;
        }

        [[nodiscard]] std::uint16_t getMaxStackSize() const
        {
            return this->maxStackSize;
        }
};

class BlockItem : public Item
{
    public:
        BlockItem(const NamespaceIdentifier& _identifier, const std::string& _displayName, const TextureId texId, const std::string& _lore) :
            Item(_identifier, _displayName, _lore, texId, 64)
        {}

        void useOn() override
        {
            // Place block
        }
};

class ADurableItem : public Item
{
    protected:
        bool unbreakable;
        std::uint16_t maxDurability;

    public:
        ADurableItem(const NamespaceIdentifier& _identifier, const std::string& _displayName, const std::string& _lore, const TextureId texId, const std::uint16_t _maxDurability) :
            Item(_identifier, _displayName, _lore, texId, 1),
            unbreakable(false),
            maxDurability(_maxDurability)
        {}

        ADurableItem(const NamespaceIdentifier& _identifier, const std::string& _displayName, const TextureId texId, const std::string& _lore) :
            Item(_identifier, _displayName, _lore, texId, 1),
            unbreakable(true),
            maxDurability(0)
        {}

        [[nodiscard]] bool isUnbreakable() const
        {
            return unbreakable;
        }

        [[nodiscard]] std::uint16_t getMaxDurability() const
        {
            return maxDurability;
        }
};

class SwordItem : public ADurableItem
{
    std::uint16_t damage;

    public:
        SwordItem(const NamespaceIdentifier& _identifier, const std::string& _displayName, const std::string& _lore, const TextureId texId, const std::uint16_t _damage) :
            ADurableItem(_identifier, _displayName, _lore, texId, 100),
            damage(_damage)
        {}

        void use() override
        {
            // Apply damages
        }
};

class PickaxeItem : public ADurableItem
{
    std::uint16_t miningSpeed;

    public:
        PickaxeItem(const NamespaceIdentifier& _identifier, const std::string& _displayName, const std::string& _lore, const TextureId texId, const std::uint16_t _miningSpeed) :
            ADurableItem(_identifier, _displayName, _lore, texId, 100),
            miningSpeed(_miningSpeed)
        {}

        void use() override
        {
            // Break blocks
        }
};

struct ItemStack
{
    ItemId itemId = 0;
    std::uint8_t stackSize = 0;
    std::uint16_t durability = 0;
};

#endif