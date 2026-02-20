#include "ChunkMesh.h"

ChunkMesh::ChunkMesh(const ChunkPos& pos) :
    position(pos)
{
    // Empty
}

void ChunkMesh::upload(MeshData&& data)
{
    // Determine back buffer index (opposite of front)
    const uint8_t backIndex = 1 - frontBufferIndex.load(std::memory_order_acquire);

    // Store vertex count for the back buffer
    vertexCounts[backIndex] = data.size();

    // Upload to back buffer VAO/VBO
    buffers[backIndex].bind();
    buffers[backIndex].storeBlockData(data);
    buffers[backIndex].unbind();
}

void ChunkMesh::swapBuffers()
{
    const uint8_t current = frontBufferIndex.load(std::memory_order_acquire);
    frontBufferIndex.store(1 - current, std::memory_order_release);
}

void ChunkMesh::render() const
{
    const uint8_t frontIndex = frontBufferIndex.load(std::memory_order_acquire);

    if (vertexCounts[frontIndex] == 0)
        return;

    this->buffers[frontIndex].draw();
}

bool ChunkMesh::hasGeometry() const
{
    const uint8_t frontIndex = frontBufferIndex.load(std::memory_order_acquire);
    return vertexCounts[frontIndex] > 0;
}
