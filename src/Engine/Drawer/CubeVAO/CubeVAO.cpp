
#include "CubeVAO.h"

CubeVAO::CubeVAO()
{
    glGenVertexArrays(1, &this->ID);
}

CubeVAO::~CubeVAO()
{
    glDeleteVertexArrays(1, &this->ID);
}

void CubeVAO::linkVertices(std::vector<GLfloat> &vertices, std::vector<GLuint> &indices) const
{
    // Handle vertices
    this->VerticesVBO.addData(vertices);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    this->VerticesVBO.unbind();

    // Handle indices
    this->EBO.addData(indices);
    // this->EBO.unbind();
}

void CubeVAO::linkTexOffset(std::vector<GLuint> &offsets) const
{
    this->TextOffsetVBO.addData(offsets);

    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, 0, (void *)0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

    this->TextOffsetVBO.unbind();
}

void CubeVAO::linkPosOffset(std::vector<GLfloat> &offsets) const
{
    this->PosOffsetVBO.addData(offsets);

    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *)0);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 6);

    this->PosOffsetVBO.unbind();
}

void CubeVAO::linkRenderedSides(std::vector<GLint> &sides) const
{
    this->RenderedSidesVBO.addData(sides);

    glEnableVertexAttribArray(4);
    glVertexAttribIPointer(4, 4, GL_INT, sizeof(GLint), (void *)0);
    glVertexAttribDivisor(4, 1);

    this->RenderedSidesVBO.unbind();
}


void CubeVAO::bind() const
{
    glBindVertexArray(this->ID);
}

void CubeVAO::unbind() const
{
    glBindVertexArray(0);
}