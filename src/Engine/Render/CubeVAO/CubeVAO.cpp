
#include "CubeVAO.h"

CubeVAO::CubeVAO()
{
    glGenVertexArrays(1, &this->ID);
}

CubeVAO::~CubeVAO()
{
    glDeleteVertexArrays(1, &this->ID);
}

void CubeVAO::linkVertices(const std::vector<GLint> &vertices) const
{
    this->VerticesVBO.addData(vertices);
    glVertexAttribIPointer(0, 3, GL_UNSIGNED_INT, 3 * sizeof(GLint), (void*)0);
    glEnableVertexAttribArray(0);
    this->VerticesVBO.unbind();
}

void CubeVAO::linkTex(std::vector<GLfloat> &tex) const
{
    this->TexOffsetVBO.addData(tex);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(1);
    this->TexOffsetVBO.unbind();
}

void CubeVAO::bind() const
{
    glBindVertexArray(this->ID);
}

void CubeVAO::unbind() const
{
    glBindVertexArray(0);
}