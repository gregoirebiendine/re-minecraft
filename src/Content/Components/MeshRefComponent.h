#ifndef RE_MINECRAFT_MESHREFCOMPONENT_H
#define RE_MINECRAFT_MESHREFCOMPONENT_H

#include <memory>
#include <EntityMeshData.h>
#include <TextureRegistry.h>

namespace ECS
{
    struct MeshRefComponent
    {
        std::shared_ptr<EntityMeshData> mesh;
        TextureId texId;
    };
}

#endif