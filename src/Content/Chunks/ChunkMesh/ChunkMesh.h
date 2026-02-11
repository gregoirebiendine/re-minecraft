#ifndef FARFIELD_CHUNKMESH_H
#define FARFIELD_CHUNKMESH_H

#pragma once

#include <vector>
#include <atomic>

#include "Chunk.h"
#include "Vertex.h"
#include "VAO.h"

using MeshData = std::vector<PackedBlockVertex>;

class ChunkMesh {
    public:
        explicit ChunkMesh(const ChunkPos& pos);


        void upload(MeshData&& data);
        void swapBuffers();
        void render() const;

        [[nodiscard]] bool hasGeometry() const;

    private:
        ChunkPos position;

        VAO buffers[2];
        size_t vertexCounts[2]{0, 0};

        std::atomic<uint8_t> frontBufferIndex{0};
};

#endif
