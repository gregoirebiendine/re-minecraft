#include "VAO.h"

VAO::VAO()
{
    glGenVertexArrays(1, &this->ID);
}

VAO::~VAO()
{
    glDeleteVertexArrays(1, &this->ID);
}

void VAO::storeBlockData(const std::vector<Vertex> &data) const {
    this->BlockVBO.addData(data);

    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(0, 3, GL_INT, sizeof(Vertex), static_cast<void *>( nullptr ));

    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(1, 3, GL_INT, sizeof(Vertex), reinterpret_cast<void *>( offsetof(Vertex, normal) ));

    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(2, 2, GL_INT, sizeof(Vertex), reinterpret_cast<void *>( offsetof(Vertex, uv) ));

    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(Vertex), reinterpret_cast<void *>( offsetof(Vertex, texId) ));

    this->BlockVBO.unbind();
}

void VAO::storeGuiData(const std::vector<GuiVertex>& data) const
{
    this->GuiVBO.addData<GuiVertex>(data);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GuiVertex), static_cast<void *>( nullptr ));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(GuiVertex), reinterpret_cast<void *>( offsetof(GuiVertex, color) ));

    this->GuiVBO.unbind();
}

void VAO::bind() const
{
    glBindVertexArray(this->ID);
}

void VAO::unbind() const
{
    glBindVertexArray(0);
}