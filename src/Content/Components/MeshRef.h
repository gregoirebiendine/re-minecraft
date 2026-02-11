#ifndef FARFIELD_MESHREFCOMPONENT_H
#define FARFIELD_MESHREFCOMPONENT_H

#include <memory>
#include <EntityMeshData.h>
#include <TextureRegistry.h>

namespace ECS
{
    struct MeshRef
    {
        std::shared_ptr<EntityMeshData> mesh;
        TextureId texId;
    };
}

#endif