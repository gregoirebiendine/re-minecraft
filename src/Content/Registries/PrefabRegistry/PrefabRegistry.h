#ifndef RE_MINECRAFT_PREFABREGISTRY_H
#define RE_MINECRAFT_PREFABREGISTRY_H

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>

#include <json.hpp>

#include "Material.h"
#include "BlockRegistry.h"

namespace fs = std::filesystem;

using PrefabId = unsigned short; // 16bits (0..65535)
using json = nlohmann::json;

struct PrefabBlockData
{
    short x, y, z;
    Material mat;
};

struct PrefabMeta
{
    int density;
    BlockId blockBelow;
    std::vector<PrefabBlockData> blocks{};
};

class PrefabRegistry
{
    const BlockRegistry& blockRegistry;

    std::vector<PrefabMeta> prefabs;
    std::unordered_map<std::string, PrefabId> nameToPrefabId;

    PrefabId registerPrefab(const std::string& prefabFile);

    public:
        explicit PrefabRegistry(const BlockRegistry& _blockRegistry);

        const PrefabMeta& get(PrefabId id) const;
        const PrefabMeta& get(const std::string& name) const;
        PrefabId getByName(const std::string& name) const;
};


#endif //RE_MINECRAFT_PREFABREGISTRY_H