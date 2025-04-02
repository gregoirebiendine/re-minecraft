
#ifndef RE_MINECRAFT_CUBE_VAO_H
#define RE_MINECRAFT_CUBE_VAO_H

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include "VBO.h"
#include "EBO.h"

class CubeVAO {
    GLuint ID = -1;
    VBO TEST_VBO;
    VBO Vertex_VBO;
    VBO UV_VBO;
    VBO FACES_VBO;
    VBO OffsetVBO;
    EBO EBO;

    public:
        CubeVAO();
        ~CubeVAO();

        void linkVerticesAndUVs(std::vector<GLfloat> &verticesAndUvs) const;
        void linkVertices(std::vector<GLfloat> &vertices) const;
        void linkIndices(std::vector<GLuint> &indices) const;
        void linkUV(std::vector<GLfloat> &uv) const;
        void linkOffset(std::vector<GLuint> &offsets) const;
        // void linkFaces(std::vector<glm::vec3> &faces) const;
        void bind() const;
        void unbind() const;
};


#endif //RE_MINECRAFT_CUBE_VAO_H
