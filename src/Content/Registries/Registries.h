#ifndef FARFIELD_REGISTRIES_H
#define FARFIELD_REGISTRIES_H

#include <type_traits>
#include "BlockRegistry.h"
#include "ItemRegistry.h"
#include "TextureRegistry.h"
#include "MeshRegistry.h"
#include "PrefabRegistry.h"
#include "ItemMeshRegistry.h"

struct Registries
{
    BlockRegistry& blockRegistry;
    PrefabRegistry& prefabRegistry;
    TextureRegistry& textureRegistry;
    ItemRegistry& itemRegistry;
    MeshRegistry& meshRegistry;
    ItemMeshRegistry& itemMeshRegistry;

    Registries(
        BlockRegistry& br,
        PrefabRegistry& pr,
        TextureRegistry& tr,
        ItemRegistry& ir,
        MeshRegistry& mr,
        ItemMeshRegistry& imr
    ) :
        blockRegistry(br),
        prefabRegistry(pr),
        textureRegistry(tr),
        itemRegistry(ir),
        meshRegistry(mr),
        itemMeshRegistry(imr)
    {}

    template<typename T>
    T& get() const
    {
        if constexpr (std::is_same_v<T, BlockRegistry>)            return this->blockRegistry;
        else if constexpr (std::is_same_v<T, PrefabRegistry>)      return this->prefabRegistry;
        else if constexpr (std::is_same_v<T, TextureRegistry>)     return this->textureRegistry;
        else if constexpr (std::is_same_v<T, ItemRegistry>)        return this->itemRegistry;
        else if constexpr (std::is_same_v<T, MeshRegistry>)        return this->meshRegistry;
        else if constexpr (std::is_same_v<T, ItemMeshRegistry>)    return this->itemMeshRegistry;
        else static_assert(!sizeof(T), "Unknown registry type");
    }
};

#endif