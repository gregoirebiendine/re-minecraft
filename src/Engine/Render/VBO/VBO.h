#ifndef FARFIELD_VBO_H
#define FARFIELD_VBO_H

#define GLM_ENABLE_EXPERIMENTAL

#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "VAOVertices.h"

class VBO {
    GLuint ID{};

    public:
        VBO();
        ~VBO();

        template<typename T>
        void addData(const std::vector<T> &v) const
        {
            this->bind();
            glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(v.size() * sizeof(T)), v.data(), GL_STATIC_DRAW);
        }

        void bind() const;
        void unbind() const;
};

#endif //VBO_H
