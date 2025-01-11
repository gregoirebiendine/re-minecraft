
#include "CubeVAO.h"

CubeVAO::CubeVAO()
{
    glGenVertexArrays(1, &this->ID);
}

CubeVAO::~CubeVAO()
{
    glDeleteVertexArrays(1, &this->ID);
}

void CubeVAO::linkVertices(std::vector<GLfloat> &vertices) const
{
    this->Vertex_VBO.addData(vertices);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(0);
    this->Vertex_VBO.unbind();
}

void CubeVAO::linkUV(std::vector<GLfloat> &uv) const
{
    this->UV_VBO.addData(uv);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(1);
    this->UV_VBO.unbind();
}

void CubeVAO::bind() const
{
    glBindVertexArray(this->ID);
}

void CubeVAO::unbind() const
{
    glBindVertexArray(0);
}
