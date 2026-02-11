#include "EntityMeshData.h"

void EntityMeshData::upload(const std::vector<EntityVertex>& vertices)
{
    this->vao.bind();

    this->vao.storeEntityMeshData(vertices);
    this->vertexCount = static_cast<GLsizei>(vertices.size());

    this->vao.unbind();
}

void EntityMeshData::render() const
{
    this->vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, this->vertexCount);
    this->vao.unbind();
}
