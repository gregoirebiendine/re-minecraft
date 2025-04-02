#ifndef RE_MINECRAFT_CHUNK_H
#define RE_MINECRAFT_CHUNK_H

#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>
#include <cmath>
#include <array>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "CubeVAO.h"
#include "VAO.h"

class Chunk {
    CubeVAO VAO;

    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;
    std::vector<uint32_t> UVoffsets;

    public:
        Chunk();

        void bind() const;
        void draw() const;
};

#endif //RE_MINECRAFT_CHUNK_H
