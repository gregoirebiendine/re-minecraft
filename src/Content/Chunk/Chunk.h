#ifndef RE_MINECRAFT_CHUNK_H
#define RE_MINECRAFT_CHUNK_H

#include <cmath>
#include <iostream>
#include <glm/glm.hpp>
#include "CubeVAO.h"
#include "VAO.h"
#include "VBO.h"

// UV MAPPING :
// Left lower, Right lower, Right upper, Left lower, Right upper, Left upper

// Math relation :
// x = (index%NumTiles)/NumTiles // (index%NumTiles)*(1/NumTiles)
// y = 1.0 - (round(1/NumTiles) + 1) * (1/NumTiles)

class Chunk {
    CubeVAO VAO;

    std::vector<GLfloat> vertices;
    std::vector<GLfloat> uv;
    std::vector<GLuint> indices;

    public:
        Chunk();

        void bind() const;
        void draw() const;
};

#endif //RE_MINECRAFT_CHUNK_H
