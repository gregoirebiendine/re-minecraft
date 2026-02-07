#include "VBO.h"

VBO::VBO()
{
    glGenBuffers(1, &this->ID);
}

VBO::~VBO()
{
    glDeleteBuffers(1, &this->ID);
}

void VBO::bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, this->ID);
}

void VBO::unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}