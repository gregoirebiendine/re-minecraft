#ifndef RE_MINECRAFT_MESHREGISTRY_H
#define RE_MINECRAFT_MESHREGISTRY_H

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

#include "EntityMeshData.h"
#include "Vertex.h"

class MeshRegistry
{
    std::unordered_map<std::string, std::shared_ptr<EntityMeshData>> cache;

    public:
        MeshRegistry();

        std::shared_ptr<EntityMeshData> load(const std::string& name, const std::vector<EntityVertex>& vertices);
        std::shared_ptr<EntityMeshData> get(const std::string& name) const;

        void unload(const std::string& name);
        void clear();
};


#endif