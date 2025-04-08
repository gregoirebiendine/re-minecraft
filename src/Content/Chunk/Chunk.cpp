#include "Chunk.h"

#include <map>

constexpr uint16_t MAX_BLOCK = 16*16*16;
constexpr uint8_t CHUNK_SIZE = 16;

std::map<uint8_t, std::array<uint8_t, 6>> blockFaces = {
    {0, {0, 0, 0, 0, 0, 0}},
    {1, {1, 1, 1, 1, 2, 0}},
    {2, {2, 2, 2, 2, 2, 2}},
};

Chunk::Chunk() :
    data(MAX_BLOCK, 0)
{
    // data = {0, 1, 2};

    // Get 3D coords from 1D index
    // const int index = 2047;
    // int x = index / (CHUNK_SIZE * CHUNK_SIZE);
    // int y = (index / CHUNK_SIZE) % CHUNK_SIZE;
    // int z = index % CHUNK_SIZE;

    // const int texOffset = 1;
    // const glm::vec2 computedTexOffset = glm::vec2(0.25f * (texOffset % 4), floor(texOffset / 4) / 4);

    for (int i = 0; i < MAX_BLOCK; i++) {
        const int x = i / (CHUNK_SIZE * CHUNK_SIZE);
        const int y = (i / CHUNK_SIZE) % CHUNK_SIZE;
        const int z = i % CHUNK_SIZE;
        const std::array<uint8_t, 6> faces = blockFaces[data[i]];

        this->vertices.insert(this->vertices.end(), {
            1 + x, y, 1 + z,   x, y, 1 + z,   x, 1 + y, 1 + z,   1 + x, 1 + y, 1 + z,       // Front face
            x, y, z,   1 + x, y, z,   1 + x, 1 + y, z,   x, 1 + y, z,                       // Back face
            x, y, 1 + z,   x, y, z,   x, 1 + y, z,   x, 1 + y, 1 + z,                       // Left face
            1 + x, y, z,   1 + x, y, 1 + z,   1 + x, 1 + y, 1 + z,   1 + x, 1 + y, 0 + z,   // Right face
            x, 1 + y, z,   1 + x, 1 + y, z,   1 + x, 1 + y, 1 + z,   x, 1 + y, 1 + z,       // Top face
            x, y, z,   x, y, 1 + z,   1 + x, y, 1 + z,   1 + x, y, z,                       // Bottom face
        });

        for (int k = (i * 24); k < (i * 24) + 24; k+=4) {
            this->indices.insert(this->indices.end(), {
                k, k+1, k+2, k, k+2, k+3
            });
        }

        for (int j = 0; j < 6; j++) {
            const glm::vec2 texOffset = glm::vec2(0.25f * (faces[j] % 4), floor(faces[j] / 4) / 4);
            this->tex.insert(this->tex.end(), {
                0.0f + texOffset.x, 0.0f + texOffset.y, 0.25f + texOffset.x, 0.0f + texOffset.y, 0.25f + texOffset.x, 0.25f + texOffset.y, 0.0f + texOffset.x, 0.25f + texOffset.y,
            });
        }
    }

    // this->vertices = {
    //     0, 0, 0,   1, 0, 0,   1, 1, 0,   0, 1, 0,       // Front face
    //     0, 0, 1,   0, 0, 0,   0, 1, 0,   0, 1, 1,       // Left face
    //     1, 0, 1,   0, 0, 1,   0, 1, 1,   1, 1, 1,       // Back face
    //     1, 0, 0,   1, 0, 1,   1, 1, 1,   1, 1, 0,       // Right face
    //     0, 1, 0,   1, 1, 0,   1, 1, 1,   0, 1, 1,       // Top face
    //     0, 0, 0,   0, 0, 1,   1, 0, 1,   1, 0, 0,       // Bottom face
    // };

    // this->vertices = {
    //     1 + x, y, 1 + z,   x, y, 1 + z,   x, 1 + y, 1 + z,   1 + x, 1 + y, 1 + z,       // Front face
    //     x, y, z,   1 + x, y, z,   1 + x, 1 + y, z,   x, 1 + y, z,                       // Back face
    //     x, y, 1 + z,   x, y, z,   x, 1 + y, z,   x, 1 + y, 1 + z,                       // Left face
    //     1 + x, y, z,   1 + x, y, 1 + z,   1 + x, 1 + y, 1 + z,   1 + x, 1 + y, 0 + z,   // Right face
    //     x, 1 + y, z,   1 + x, 1 + y, z,   1 + x, 1 + y, 1 + z,   x, 1 + y, 1 + z,       // Top face
    //     x, y, z,   x, y, 1 + z,   1 + x, y, 1 + z,   1 + x, y, z,                       // Bottom face
    // };

    // this->indices = {
    //     0, 1, 2, 0, 2, 3,
    //     4, 5, 6, 4, 6, 7,
    //     8, 9, 10, 8, 10, 11,
    //     12, 13, 14, 12, 14, 15,
    //     16, 17, 18, 16, 18, 19,
    //     20, 21, 22, 20, 22, 23,
    //     // i, i+1, i+2, i, i+2, i+3, ((i+=4))
    // };

    // this->tex = {
    //     0.0f + computedTexOffset.x, 0.0f + computedTexOffset.y, 0.25f + computedTexOffset.x, 0.0f + computedTexOffset.y, 0.25f + computedTexOffset.x, 0.25f + computedTexOffset.y, 0.0f + computedTexOffset.x, 0.25f + computedTexOffset.y,
    //     0.0f, 0.0f, 0.25f, 0.0f, 0.25f, 0.25f, 0.0f, 0.25f,
    //     0.0f, 0.0f, 0.25f, 0.0f, 0.25f, 0.25f, 0.0f, 0.25f,
    //     0.0f, 0.0f, 0.25f, 0.0f, 0.25f, 0.25f, 0.0f, 0.25f,
    //     0.0f, 0.0f, 0.25f, 0.0f, 0.25f, 0.25f, 0.0f, 0.25f,
    //     0.0f, 0.0f, 0.25f, 0.0f, 0.25f, 0.25f, 0.0f, 0.25f,
    // };

    // Bind VAO
    this->VAO.bind();

    // Link datas to VA0 before rendering
    this->VAO.linkVertices(this->vertices, this->indices);
    this->VAO.linkTex(this->tex);

    // Unbind VAO
    this->VAO.unbind();
}

void Chunk::bind() const {
    this->VAO.bind();
}

void Chunk::draw() const
{
    // Bind VAO
    this->bind();

    // Draw instances (faces)
//    glDrawElementsInstanced(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0, this->renderedSides.size());

    glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
}
