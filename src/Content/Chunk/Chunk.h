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
    // std::array<uint16_t, 16*16*16> cubes;
    std::vector<uint8_t> cubes;

    CubeVAO VAO;
    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;
    std::vector<GLuint> texOffsets;
    std::vector<GLfloat> posOffsets;
    std::vector<GLint> renderedSides;

    public:
        Chunk();

        void bind() const;
        void draw() const;
};

#endif //RE_MINECRAFT_CHUNK_H
