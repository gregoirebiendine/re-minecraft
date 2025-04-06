#ifndef RE_MINECRAFT_CHUNK_H
#define RE_MINECRAFT_CHUNK_H

#include <iostream>
#include <cmath>
#include <array>
#include <iostream>
#include <glm/glm.hpp>

#include "CubeVAO.h"
#include "VAO.h"

class Chunk {
    std::vector<uint8_t> data;

    CubeVAO VAO;
    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;
    std::vector<GLfloat> tex;
    // std::vector<GLuint> texOffsets;
    // std::vector<GLfloat> posOffsets;
    // std::vector<GLint> renderedSides;

    public:
        Chunk();

        void bind() const;
        void draw() const;
};

#endif //RE_MINECRAFT_CHUNK_H
