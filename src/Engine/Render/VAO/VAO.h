
#ifndef RE_MINECRAFT_CUBE_VAO_H
#define RE_MINECRAFT_CUBE_VAO_H

#include "glad/glad.h"
#include "VBO.h"

class VAO {
    GLuint ID = -1;
    VBO verticesVBO;
    VBO uvsVBO;

    public:
        VAO();
        ~VAO();

        void linkVertices(const std::vector<GLint> &vertices) const;
        void linkUvs(const std::vector<GLfloat> &uvs) const;

        void bind() const;
        void unbind() const;
};


#endif //RE_MINECRAFT_CUBE_VAO_H
