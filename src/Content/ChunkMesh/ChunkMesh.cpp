#include "ChunkMesh.h"
#include "World.h"

ChunkMesh::ChunkMesh(const ChunkPos& pos) :
    position(pos)
{
    // Empty
}

void ChunkMesh::upload(MeshData&& data)
{
    this->vertexCount = data.size();

    this->VAO.bind();
    this->VAO.storeBlockData(data);
    this->VAO.unbind();
}

void ChunkMesh::render() const
{
    if (vertexCount == 0)
        return;

    this->VAO.bind();
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertexCount));
    this->VAO.unbind();
}
