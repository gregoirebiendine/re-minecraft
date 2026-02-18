#ifndef FARFIELD_INVENTORY_H
#define FARFIELD_INVENTORY_H

#include <cstdint>
#include <array>
#include <utility>

#include "Item.h"

namespace ECS
{
    struct Hotbar
    {
        std::array<ItemStack, 9> items;
    };

    struct InternalInventory
    {
        std::array<ItemStack, 36> items;
    };

    struct Equipments
    {
        std::array<ItemStack, 4> armor;
        ItemStack rightHand;
    };
}

#endif