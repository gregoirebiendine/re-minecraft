#ifndef RE_MINECRAFT_ENTITYMESHDATA_H
#define RE_MINECRAFT_ENTITYMESHDATA_H

#include <vector>
#include <glad/glad.h>

#include "VAO.h"
#include "Vertex.h"

class EntityMeshData
{
    VAO vao;
    GLsizei vertexCount = 0;

    public:
        EntityMeshData() = default;

        EntityMeshData(const EntityMeshData&) = delete;
        EntityMeshData& operator=(const EntityMeshData&) = delete;
        EntityMeshData(EntityMeshData&&) = default;
        EntityMeshData& operator=(EntityMeshData&&) = default;

        void upload(const std::vector<EntityVertex>& vertices);
        void render() const;
};

#endif