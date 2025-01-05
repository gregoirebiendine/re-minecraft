
#include "VAO.h"

VAO::VAO()
{
    glGenVertexArrays(1, &this->ID);
}

VAO::~VAO()
{
    glDeleteVertexArrays(1, &this->ID);
}

void VAO::linkAttrib(VBO &VBO, GLuint layout, GLint size, GLenum type, GLsizei stride, void *offset) const
{
    VBO.bind();
    glVertexAttribPointer(layout, size, type, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(layout);
    VBO.unbind();
}

void VAO::linkSquareAttrib(VBO &VBO) const {
    this->linkAttrib(VBO, 0, 3, GL_FLOAT, 5 * sizeof(float), 0);
    this->linkAttrib(VBO, 1, 2, GL_FLOAT, 5 * sizeof(float), (void *)(3 * sizeof(float)));
}

void VAO::bind() const
{
    glBindVertexArray(this->ID);
}

void VAO::unbind() const
{
    glBindVertexArray(0);
}
