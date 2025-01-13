
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
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    this->Vertex_VBO.unbind();
}

void CubeVAO::linkUV(std::vector<GLfloat> &uv) const
{
    this->UV_VBO.addData(uv);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glVertexAttribDivisor(1, 0);
    this->UV_VBO.unbind();
}

void CubeVAO::linkFaces(std::vector<glm::vec3> &faces) const
{
    this->FACES_VBO.addData(faces);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glVertexAttribDivisor(1, 1);
    this->FACES_VBO.unbind();
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