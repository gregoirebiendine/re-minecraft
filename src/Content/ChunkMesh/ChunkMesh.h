#ifndef RE_MINECRAFT_CHUNKMESH_H
#define RE_MINECRAFT_CHUNKMESH_H

#pragma once

class World; // forward declaration

#include <glad/glad.h>
#include <vector>

#include "Chunk.h"
#include "VAO.h"

class ChunkMesh
{
    public:
        ChunkMesh() = default;
        ~ChunkMesh() = default;

        void rebuild(Chunk& chunk, const World& world);
        void render() const;

    private:
        VAO VAO;
        std::vector<GLint> vertices;
        std::vector<GLfloat> uvs;
        std::vector<GLfloat> normals;
        std::vector<GLint> textureIndexes;
};


#endif //RE_MINECRAFT_CHUNKMESH_H