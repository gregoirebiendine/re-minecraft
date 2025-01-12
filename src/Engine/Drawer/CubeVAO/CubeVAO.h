
#ifndef RE_MINECRAFT_CUBE_VAO_H
#define RE_MINECRAFT_CUBE_VAO_H

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "VBO.h"
#include "EBO.h"

class CubeVAO {
    GLuint ID = -1;
    VBO Vertex_VBO;
    VBO UV_VBO;
    EBO EBO;

    public:
        CubeVAO();
        ~CubeVAO();

        void linkVertices(std::vector<GLfloat> &vertices) const;
        void linkUV(std::vector<GLfloat> &uv) const;
        void linkIndices(std::vector<GLuint> &indices) const;
        void bind() const;
        void unbind() const;
};


#endif //RE_MINECRAFT_CUBE_VAO_H
