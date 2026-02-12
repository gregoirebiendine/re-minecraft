#include "MeshRegistry.h"
#include "ZombieMesh.h"
#include "PlayerMesh.h"

MeshRegistry::MeshRegistry()
{
    this->load("zombie", ZOMBIE_MESH);
    this->load("player", PLAYER_MESH);
}

std::shared_ptr<EntityMeshData> MeshRegistry::load(const std::string& name, const std::vector<EntityVertex>& vertices)
{
    if (const auto it = this->cache.find(name); it != this->cache.end())
        return it->second;

    auto mesh = std::make_shared<EntityMeshData>();
    mesh->upload(vertices);
    this->cache[name] = mesh;

    return mesh;
}

std::shared_ptr<EntityMeshData> MeshRegistry::get(const std::string& name) const
{
    if (const auto it = cache.find(name); it != cache.end())
        return it->second;
    return nullptr;
}

void MeshRegistry::unload(const std::string& name)
{
    cache.erase(name);
}

void MeshRegistry::clear()
{
    cache.clear();
}
