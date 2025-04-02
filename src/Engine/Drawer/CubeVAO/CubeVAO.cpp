
#include "CubeVAO.h"

CubeVAO::CubeVAO()
{
    glGenVertexArrays(1, &this->ID);
}

CubeVAO::~CubeVAO()
{
    glDeleteVertexArrays(1, &this->ID);
}

void CubeVAO::linkVerticesAndUVs(std::vector<GLfloat> &verticesAndUvs) const
{
    this->TEST_VBO.addData(verticesAndUvs);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    this->TEST_VBO.unbind();
}

void CubeVAO::linkVertices(std::vector<GLfloat> &vertices) const
{
    this->Vertex_VBO.addData(vertices);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    this->Vertex_VBO.unbind();
}

void CubeVAO::linkUV(std::vector<GLfloat> &uv) const
{
    this->UV_VBO.addData(uv);

    // glEnableVertexAttribArray(1);
    // glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *)0);
    // glVertexAttribDivisor(1, 1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

    this->UV_VBO.unbind();
}

void CubeVAO::linkOffset(std::vector<GLuint> &offsets) const
{
    this->OffsetVBO.addData(offsets);

    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, 0, (void *)0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

    this->OffsetVBO.unbind();
}

void CubeVAO::linkIndices(std::vector<GLuint> &indices) const
{
    this->EBO.addData(indices);
}

void CubeVAO::bind() const
{
    glBindVertexArray(this->ID);
}

void CubeVAO::unbind() const
{
    glBindVertexArray(0);
}