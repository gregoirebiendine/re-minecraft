
#ifndef RE_MINECRAFT_CUBE_VAO_H
#define RE_MINECRAFT_CUBE_VAO_H

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "VBO.h"

class CubeVAO {
    GLuint ID = -1;
    VBO VerticesVBO;
    VBO TexOffsetVBO;

    public:
        CubeVAO();
        ~CubeVAO();

        void linkVertices(std::vector<GLuint> &vertices) const;
        void linkTex(std::vector<GLfloat> &tex) const;

        void bind() const;
        void unbind() const;
};


#endif //RE_MINECRAFT_CUBE_VAO_H
