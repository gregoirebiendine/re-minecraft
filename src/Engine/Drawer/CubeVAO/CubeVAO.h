
#ifndef RE_MINECRAFT_CUBE_VAO_H
#define RE_MINECRAFT_CUBE_VAO_H

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "VBO.h"
#include "EBO.h"

class CubeVAO {
    GLuint ID = -1;
    VBO VerticesVBO;
    VBO TexOffsetVBO;
    EBO EBO;

    public:
        CubeVAO();
        ~CubeVAO();

        void linkVertices(std::vector<GLuint> &vertices, std::vector<GLuint> &indices) const;
        void linkTex(std::vector<GLfloat> &tex) const;

        void bind() const;
        void unbind() const;
};


#endif //RE_MINECRAFT_CUBE_VAO_H
