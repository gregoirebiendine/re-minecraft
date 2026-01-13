
#include "VAO.h"

VAO::VAO()
{
    glGenVertexArrays(1, &this->ID);
}

VAO::~VAO()
{
    glDeleteVertexArrays(1, &this->ID);
}

template<typename T = GLfloat, int K = GL_FLOAT>
void VAO::addData(const std::vector<T> &vertices, const int index, const int size)
{
    this->bind();
    this->VBOs[index].addData<T>(vertices);

    glEnableVertexAttribArray(index);
    if (std::is_same_v<T, GLfloat>)
        glVertexAttribPointer(index, size, K, GL_FALSE, size * sizeof(T), static_cast<void *>(nullptr));
    else if (std::is_same_v<T, GLint> || std::is_same_v<T, GLuint>)
        glVertexAttribIPointer(index, size, K, size * sizeof(T), static_cast<void *>(nullptr));

    this->VBOs[index].unbind();
}

void VAO::storeBlockData(const std::vector<Vertex> &data) const {
    this->BlockVBO.storeBlockData(data);

    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(0, 3, GL_INT, sizeof(Vertex), static_cast<void *>( nullptr ));

    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(1, 3, GL_INT, sizeof(Vertex), reinterpret_cast<void *>( offsetof(Vertex, normal) ));

    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(2, 2, GL_INT, sizeof(Vertex), reinterpret_cast<void *>( offsetof(Vertex, uv) ));

    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(Vertex), reinterpret_cast<void *>( offsetof(Vertex, texId) ));

    this->BlockVBO.unbind();
}

void VAO::bind() const
{
    glBindVertexArray(this->ID);
}

void VAO::unbind() const
{
    glBindVertexArray(0);
}


// Templates
template void VAO::addData<GLfloat, GL_FLOAT>(const std::vector<GLfloat> &vertices, int index, int size);
template void VAO::addData<GLint, GL_INT>(const std::vector<GLint> &vertices, int index, int size);