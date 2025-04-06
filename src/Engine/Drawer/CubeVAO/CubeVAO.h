
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
    VBO PosOffsetVBO;
    VBO RenderedSidesVBO;
    EBO EBO;

    public:
        CubeVAO();
        ~CubeVAO();

        void linkVertices(std::vector<GLfloat> &vertices, std::vector<GLuint> &indices) const;
        void linkTex(std::vector<GLfloat> &tex) const;

        void linkTexOffset(std::vector<GLuint> &offsets) const;
        void linkPosOffset(std::vector<GLfloat> &offsets) const;
        void linkRenderedSides(std::vector<GLint> &sides) const;

        void bind() const;
        void unbind() const;
};


#endif //RE_MINECRAFT_CUBE_VAO_H
