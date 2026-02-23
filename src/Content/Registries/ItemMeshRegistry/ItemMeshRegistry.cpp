#include "ItemMeshRegistry.h"
#include "TextureExtruder.h"

ItemMeshRegistry::ItemMeshRegistry(const TextureRegistry &textureRegistry, const ItemRegistry &itemRegistry)
{
    const auto& items = itemRegistry.getAll();

    for (const auto& itemId : items) {
        const Item& item = itemRegistry.get(itemId);
        const TextureId& texId = item.getTextureId();
        const stbi_uc* texData = textureRegistry.getTextureData(texId);
        const std::string& name = item.getIdentifier().getFullIdentifier();

        this->registerItemMesh(name, texData);
    }
}

ItemMeshId ItemMeshRegistry::registerItemMesh(const std::string &fullIdentifier, const stbi_uc* texData)
{
    // Prevent duplication
    const auto it = this->nameToId.find(fullIdentifier);
    if (it != this->nameToId.end())
        return it->second;

    // Enforce max ID range
    if (this->meshes.size() >= std::numeric_limits<ItemMeshId>::max())
        throw std::runtime_error("[ItemMeshRegistry::registerItemMesh] ID overflow, maximum ItemMesh storage reached");

    const auto id = static_cast<ItemMeshId>(this->meshes.size());
    const auto mesh = TextureExtruder::generate(texData, 32, 32);
    auto meshData = std::make_shared<EntityMeshData>();

    meshData->upload(mesh);
    this->meshes.emplace_back(meshData);
    this->nameToId.emplace(fullIdentifier, id);

    return id;
}

std::shared_ptr<EntityMeshData> ItemMeshRegistry::get(const ItemId id) const
{
    if (id >= this->meshes.size())
        throw std::out_of_range("[ItemMeshRegistry::get] Out of range ID : " + std::to_string(id));
    return this->meshes[id];
}

std::shared_ptr<EntityMeshData> ItemMeshRegistry::get(const std::string &name) const
{
    const auto& id = this->getIdByName(name);

    if (id >= this->meshes.size())
        throw std::out_of_range("[ItemMeshRegistry::get] Out of range ID : " + std::to_string(id));
    return this->meshes[id];
}

ItemId ItemMeshRegistry::getIdByName(const std::string& name) const
{
    if (!this->nameToId.contains(name))
        return 0;
    return this->nameToId.at(name);
}