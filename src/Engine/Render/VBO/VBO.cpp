
#include "VBO.h"

VBO::VBO()
{
    glGenBuffers(1, &this->ID);
}

VBO::~VBO()
{
    glDeleteBuffers(1, &this->ID);
}

template<typename T = GLfloat>
void VBO::addData(const std::vector<T> &v) const {
    this->bind();
    glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(T), v.data(), GL_STATIC_DRAW);
}

void VBO::storeBlockData(const std::vector<Vertex> &v) const {
    this->bind();
    glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(Vertex), v.data(), GL_STATIC_DRAW);
}

void VBO::bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, this->ID);
}

void VBO::unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

template void VBO::addData<GLfloat>(const std::vector<GLfloat> &v) const;
template void VBO::addData<GLint>(const std::vector<GLint> &v) const;