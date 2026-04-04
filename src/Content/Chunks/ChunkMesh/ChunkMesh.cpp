#include "ChunkMesh.h"

ChunkMesh::ChunkMesh(const ChunkPos& pos) :
    position(pos)
{}

void ChunkMesh::upload(MeshData&& data)
{
    // Determine back buffer index (opposite of front)
    const uint8_t backIndex = 1 - this->frontBufferIndex.load(std::memory_order_acquire);

    // Store vertex count for the back buffer
    this->vertexCounts[backIndex] = data.size();

    // Upload to back buffer VAO/VBO
    this->buffers[backIndex].bind();
    this->buffers[backIndex].storeBlockData(data);
    this->buffers[backIndex].unbind();
}

void ChunkMesh::swapBuffers()
{
    const uint8_t current = this->frontBufferIndex.load(std::memory_order_acquire);
    this->frontBufferIndex.store(1 - current, std::memory_order_release);
}

void ChunkMesh::render() const
{
    const uint8_t frontIndex = frontBufferIndex.load(std::memory_order_acquire);

    if (this->vertexCounts[frontIndex] == 0)
        return;

    this->buffers[frontIndex].draw();
}

bool ChunkMesh::hasGeometry() const
{
    const uint8_t frontIndex = this->frontBufferIndex.load(std::memory_order_acquire);
    return this->vertexCounts[frontIndex] > 0;
}
