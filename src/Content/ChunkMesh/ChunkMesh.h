#ifndef RE_MINECRAFT_CHUNKMESH_H
#define RE_MINECRAFT_CHUNKMESH_H

#pragma once

#include <vector>

#include "Chunk.h"
#include "Vertex.h"
#include "VAO.h"

using MeshData = std::vector<Vertex>;

class ChunkMesh {
    public:
        explicit ChunkMesh(const ChunkPos& pos);

        void upload(MeshData&& data);
        void render() const;

    private:
        ChunkPos position;
        VAO VAO;
        size_t vertexCount{0};
};

#endif //RE_MINECRAFT_CHUNKMESH_H