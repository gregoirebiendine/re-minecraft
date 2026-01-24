#include "VAO.h"

VAO::VAO()
{
    glGenVertexArrays(1, &this->ID);
}

VAO::~VAO()
{
    glDeleteVertexArrays(1, &this->ID);
}

void VAO::storeBlockData(const std::vector<PackedBlockVertex> &data) const {
    this->vbo.addData(data);

    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(PackedBlockVertex), reinterpret_cast<void*>(offsetof(PackedBlockVertex, data1)));

    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(PackedBlockVertex), reinterpret_cast<void*>(offsetof(PackedBlockVertex, data2)));

    this->vbo.unbind();
}

void VAO::storeGuiData(const std::vector<GuiVertex>& data) const
{
    this->vbo.addData<GuiVertex>(data);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GuiVertex), static_cast<void *>( nullptr ));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(GuiVertex), reinterpret_cast<void *>( offsetof(GuiVertex, color) ));

    this->vbo.unbind();
}

void VAO::storeOutlineData(const std::vector<GLfloat>& data) const
{
    this->vbo.addData<GLfloat>(data);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), static_cast<void *>( nullptr ));

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