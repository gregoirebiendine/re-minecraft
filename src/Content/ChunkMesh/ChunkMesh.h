#ifndef RE_MINECRAFT_CHUNKMESH_H
#define RE_MINECRAFT_CHUNKMESH_H

#pragma once

class World; // forward declaration

#include <vector>

#include <glad/glad.h>

#include "Chunk.h"
#include "VAO.h"
#include "Vertex.h"

using ChunkMeshData = std::vector<Vertex>;

class ChunkMesh
{
    public:
        ChunkPos position;
        ChunkMeshData meshData;

        ChunkMesh(const ChunkPos& cp);

        void rebuild(Chunk& chunk, const World& world);
        void upload() const;
        void render() const;

    private:
        VAO VAO;

        static void createFace(
            std::vector<Vertex>& data,
            const glm::ivec3& v0,
            const glm::ivec3& v1,
            const glm::ivec3& v2,
            const glm::ivec3& v3,
            const glm::ivec3& normals,
            const uint16_t& texId
        );

        // Statics
        static std::tuple<GLint, GLint, GLint> coords(int index);
};


#endif //RE_MINECRAFT_CHUNKMESH_H