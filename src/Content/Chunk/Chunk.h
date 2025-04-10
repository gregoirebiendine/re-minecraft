#ifndef RE_MINECRAFT_CHUNK_H
#define RE_MINECRAFT_CHUNK_H

#include <iostream>
#include <cmath>
#include <array>
#include <iostream>
#include <glm/glm.hpp>
#include <map>

#include "CubeVAO.h"
#include "VAO.h"

class Chunk {
    uint8_t chunkData[16][16][16]{};

    glm::ivec3 chunkOffset{};

    CubeVAO VAO;
    std::vector<GLuint> vertices;
    std::vector<GLfloat> tex;

    public:
        Chunk(glm::ivec3 offset);

        void bind() const;
        void draw() const;
};

#endif //RE_MINECRAFT_CHUNK_H
