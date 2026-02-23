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
    TextureRegistry* textureRegistry{nullptr};
    ItemRegistry* itemRegistry{nullptr};
    MeshRegistry* meshRegistry{nullptr};
    ItemMeshRegistry* itemMeshRegistry{nullptr};

    Registries(BlockRegistry& br, PrefabRegistry& pr) :
        blockRegistry(br), prefabRegistry(pr)
    {}

    void setGL(TextureRegistry& tr, ItemRegistry& ir, MeshRegistry& mr, ItemMeshRegistry& imr)
    {
        this->textureRegistry = &tr;
        this->itemRegistry = &ir;
        this->meshRegistry = &mr;
        this->itemMeshRegistry = &imr;
    }

    template<typename T>
    T& get() const
    {
        if constexpr (std::is_same_v<T, BlockRegistry>)            return blockRegistry;
        else if constexpr (std::is_same_v<T, PrefabRegistry>)      return prefabRegistry;
        else if constexpr (std::is_same_v<T, TextureRegistry>)     return *textureRegistry;
        else if constexpr (std::is_same_v<T, ItemRegistry>)        return *itemRegistry;
        else if constexpr (std::is_same_v<T, MeshRegistry>)        return *meshRegistry;
        else if constexpr (std::is_same_v<T, ItemMeshRegistry>)    return *itemMeshRegistry;
        else static_assert(!sizeof(T), "Unknown registry type");
    }
};

#endif