
#include "EBO.h"

EBO::EBO()
{
    glGenBuffers(1, &this->ID);
}

EBO::~EBO()
{
    glDeleteBuffers(1, &this->ID);
}

void EBO::addData(const std::vector<GLuint> &indices) const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
}

void EBO::bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ID);
}

void EBO::unbind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}