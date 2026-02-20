#include "VAO.h"

VAO::VAO()
{
    glGenVertexArrays(1, &this->ID);
}

VAO::~VAO()
{
    glDeleteVertexArrays(1, &this->ID);
}

void VAO::storeBlockData(const std::vector<PackedBlockVertex> &data)
{
    this->size = static_cast<GLsizei>(data.size());

    this->vbo.addData(data);

    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(PackedBlockVertex), reinterpret_cast<void*>(offsetof(PackedBlockVertex, data1)));

    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(PackedBlockVertex), reinterpret_cast<void*>(offsetof(PackedBlockVertex, data2)));

    this->vbo.unbind();
}

void VAO::storeGuiData(const std::vector<GuiVertex>& data)
{
    this->size = static_cast<GLsizei>(data.size());

    this->vbo.addData<GuiVertex>(data);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GuiVertex), static_cast<void *>( nullptr ));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GuiVertex), reinterpret_cast<void *>( offsetof(GuiVertex, uv) ));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(GuiVertex), reinterpret_cast<void *>( offsetof(GuiVertex, color) ));

    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 1, GL_INT, sizeof(GuiVertex), reinterpret_cast<void *>( offsetof(GuiVertex, texId) ));

    this->vbo.unbind();
}

void VAO::storeOutlineData(const std::vector<GLfloat>& data)
{
    this->size = static_cast<GLsizei>(data.size());

    this->vbo.addData<GLfloat>(data);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), static_cast<void *>( nullptr ));

    this->vbo.unbind();
}

void VAO::storeEntityMeshData(const std::vector<EntityVertex>& data)
{
    this->size = static_cast<GLsizei>(data.size());

    this->vbo.addData<EntityVertex>(data);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(EntityVertex), reinterpret_cast<void*>(offsetof(EntityVertex, position)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(EntityVertex), reinterpret_cast<void *>( offsetof(EntityVertex, normal) ));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(EntityVertex), reinterpret_cast<void *>( offsetof(EntityVertex, uv) ));

    this->vbo.unbind();
}

void VAO::storeMsdfData(const std::vector<MSDFVertex>& data)
{
    this->size = static_cast<GLsizei>(data.size());

    this->vbo.addData<MSDFVertex>(data);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(MSDFVertex), reinterpret_cast<void*>(offsetof(MSDFVertex, position)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MSDFVertex), reinterpret_cast<void *>( offsetof(MSDFVertex, uv) ));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(MSDFVertex), reinterpret_cast<void *>( offsetof(MSDFVertex, color) ));

    this->vbo.unbind();
}

void VAO::bind() const
{
    glBindVertexArray(this->ID);
}

void VAO::unbind() const
{
    glBindVertexArray(0);
}

void VAO::draw() const
{
    this->bind();
    glDrawArrays(GL_TRIANGLES, 0, this->size);
    this->unbind();
}
