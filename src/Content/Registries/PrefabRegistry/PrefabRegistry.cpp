#include "PrefabRegistry.h"

PrefabRegistry::PrefabRegistry(const BlockRegistry& _blockRegistry) :
    blockRegistry(_blockRegistry)
{
    this->registerPrefab("oak_tree_1.json");
}

PrefabId PrefabRegistry::registerPrefab(const std::string& prefabFile)
{
    const auto path = fs::current_path().parent_path().string().append("/resources/data/prefabs/").append(prefabFile);
    const auto name = fs::path(prefabFile).replace_extension().string();

    std::ifstream file(path);
    json data = json::parse(file);

    // Prevent duplication
    auto it = this->nameToPrefabId.find(name);
    if (it != this->nameToPrefabId.end())
        return it->second;

    // Enforce max ID range
    if (this->prefabs.size() >= std::numeric_limits<PrefabId>::max())
        throw std::runtime_error("[PrefabRegistry] Farfield has reached its maximum Prefab storage.");

    // Check JSON format
    if (data.is_null() || !data.contains("block_below") || !data.contains("content"))
        throw std::runtime_error("[PrefabRegistry] Loaded file isn't valid : " + prefabFile);

    // Parse rules
    PrefabMeta meta{
        3,
        this->blockRegistry.getByName(data["block_below"].get<std::string>()),
        {}
    };

    // Parse content (blocks)
    for (const auto& content : data["content"])
    {
        const auto blockName = content["block"].get<std::string>();
        const auto rotation = content["rotation"].get<int>();
        const auto mat = Material::pack(this->blockRegistry.getByName(blockName), rotation);

        if (!content.contains("command"))
        {
            const auto [x,y,z] = content["position"].get<std::array<short, 3>>();

            meta.blocks.push_back({
                x, y, z, mat
            });
        }
        else if (content["command"].get<std::string>() == "FILL")
        {
            const auto [x1,y1,z1] = content["start"].get<std::array<short, 3>>();
            const auto [x2,y2,z2] = content["stop"].get<std::array<short, 3>>();

            if (x1 > x2 || y1 > y2 || z1 > z2)
                throw std::runtime_error("[PrefabRegistry] Invalid FILL command, coords are not in the right order : " + blockName);

            for (short z = z1; z <= z2; z++)
                for (short y = y1; y <= y2; y++)
                    for (short x = x1; x <= x2; x++)
                        meta.blocks.push_back({x, y, z, mat});
        }
        else if (content["command"].get<std::string>() == "FILL_RANGE")
        {
            const auto ranges = content["ranges"].get<std::vector<std::array<std::array<short, 3>, 2>>>();

            for (const auto& range : ranges)
            {
                const auto [x1,y1,z1] = range[0];
                const auto [x2,y2,z2] = range[1];

                if (x1 > x2 || y1 > y2 || z1 > z2)
                    throw std::runtime_error("[PrefabRegistry] Invalid FILL command, coords are not in the right order : " + blockName);

                for (short z = z1; z <= z2; z++)
                    for (short y = y1; y <= y2; y++)
                        for (short x = x1; x <= x2; x++)
                            meta.blocks.push_back({x, y, z, mat});
            }
        }
    }

    auto id = static_cast<PrefabId>(this->prefabs.size());

    this->prefabs.push_back(meta);
    this->nameToPrefabId.emplace(name, id);

    file.close();
    return id;
}

const PrefabMeta& PrefabRegistry::get(const PrefabId id) const
{
    if (id >= this->prefabs.size())
        throw std::out_of_range("Out of range PrefabId. This means this prefab is not registered in the Engine");
    return this->prefabs[id];
}

const PrefabMeta &PrefabRegistry::get(const std::string &name) const {
    const auto it = this->nameToPrefabId.find(name);

    if (it == this->nameToPrefabId.end())
        throw std::runtime_error("[PrefabRegistry] PrefabId does not exist.");
    return this->prefabs[it->second];
}

PrefabId PrefabRegistry::getByName(const std::string& name) const
{
    if (!this->nameToPrefabId.contains(name))
        return 0;
    return this->nameToPrefabId.at(name);
}