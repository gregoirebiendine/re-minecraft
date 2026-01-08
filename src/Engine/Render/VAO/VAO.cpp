
#include "VAO.h"

VAO::VAO()
{
    glGenVertexArrays(1, &this->ID);
}

VAO::~VAO()
{
    glDeleteVertexArrays(1, &this->ID);
}

void VAO::linkVertices(const std::vector<GLint> &vertices) const
{
    this->verticesVBO.addData(vertices);
    glVertexAttribIPointer(0, 3, GL_UNSIGNED_INT, 3 * sizeof(GLint), (void*)0);
    glEnableVertexAttribArray(0);
    this->verticesVBO.unbind();
}

void VAO::linkVertices(const std::vector<GLfloat> &vertices) const
{
    this->verticesVBO.addData(vertices);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    this->verticesVBO.unbind();
}

void VAO::linkUvs(const std::vector<GLfloat> &uvs) const
{
    this->uvsVBO.addData(uvs);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    this->uvsVBO.unbind();
}

void VAO::bind() const
{
    glBindVertexArray(this->ID);
}

void VAO::unbind() const
{
    glBindVertexArray(0);
}