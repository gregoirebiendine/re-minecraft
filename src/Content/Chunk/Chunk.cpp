#include "Chunk.h"

constexpr uint8_t CHUNK_SIZE = 16;
constexpr uint16_t MAX_BLOCK = CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE;

std::map<uint8_t, std::array<uint8_t, 6>> blockFaces = {
    {0, {0, 0, 0, 0, 0, 0}},    // Air
    {1, {0, 0, 0, 0, 0, 0}},    // Dirt
    {2, {1, 1, 1, 1, 2, 0}},    // Grass
    {3, {2, 2, 2, 2, 2, 2}},    // Moss
};

Chunk::Chunk()
{
    // Fill the chunk with Materials (dirt for instance)
    for (int i = 0; i < MAX_BLOCK; i++) {
        const glm::uvec3 coords = glm::uvec3(i / (CHUNK_SIZE * CHUNK_SIZE), (i / CHUNK_SIZE) % CHUNK_SIZE, i % CHUNK_SIZE);
        this->chunkData[coords.z][coords.y][coords.x] = 1;
    }

    // Fill first layer with grass for better look
    for (auto &z : this->chunkData) {
        for (uint8_t x = 0; x < CHUNK_SIZE; x++) {
            z[15][x] = 2;
        }
    }

    // Add air in top of the chunk to test robustness of the code
    this->chunkData[8][15][8] = 0;

    // Iterate over all Materials to construct blocks
    for (uint8_t z = 0; z < CHUNK_SIZE; z++) {
        for (uint8_t y = 0; y < CHUNK_SIZE; y++) {
            for (uint8_t x = 0; x < CHUNK_SIZE; x++) {
                // Skip AIR (index 0)
                if (chunkData[z][y][x] == 0)
                    continue;

                // Retrieve faces atlas indexes
                const std::array<uint8_t, 6> faces = blockFaces[chunkData[z][y][x]];

                // Left face
                if (x == 0 || (x != 0 && this->chunkData[z][y][x - 1] == 0)) {
                    const glm::vec2 t = glm::vec2(0.25f * (faces[2] % 4), floor(faces[2] / 4) / 4);

                    this->vertices.insert(this->vertices.end(), {
                        x, y, 1 + z,    x, y, z,    x, 1 + y, z,    x, y, 1 + z,    x, 1 + y, z,    x, 1 + y, 1 + z,    // Left face
                    });

                    this->tex.insert(this->tex.end(), {
                        0.0f + t.x, 0.0f + t.y,     0.25f + t.x, 0.0f + t.y,    0.25f + t.x, 0.25f + t.y,   0.0f + t.x, 0.0f + t.y,     0.25f + t.x, 0.25f + t.y,   0.0f + t.x, 0.25f + t.y,
                    });
                }

                // Right face
                if (x == CHUNK_SIZE - 1 || (x != CHUNK_SIZE - 1 && this->chunkData[z][y][x + 1] == 0)) {
                    const glm::vec2 t = glm::vec2(0.25f * (faces[3] % 4), floor(faces[3] / 4) / 4);

                    this->vertices.insert(this->vertices.end(), {
                        1 + x, y, z,    1 + x, y, 1 + z,    1 + x, 1 + y, 1 + z,    1 + x, y, z,    1 + x, 1 + y, 1 + z,    1 + x, 1 + y, z,
                    });

                    this->tex.insert(this->tex.end(), {
                        0.0f + t.x, 0.0f + t.y,     0.25f + t.x, 0.0f + t.y,    0.25f + t.x, 0.25f + t.y,   0.0f + t.x, 0.0f + t.y,     0.25f + t.x, 0.25f + t.y,   0.0f + t.x, 0.25f + t.y,
                    });
                }

                // Top face
                if (y == CHUNK_SIZE - 1 || (y != CHUNK_SIZE - 1 && this->chunkData[z][y + 1][x] == 0)) {
                    const glm::vec2 t = glm::vec2(0.25f * (faces[4] % 4), floor(faces[4] / 4) / 4);

                    this->vertices.insert(this->vertices.end(), {
                        x, 1 + y, z,    1 + x, 1 + y, z,    1 + x, 1 + y, 1 + z,    x, 1 + y, z,    1+ x, 1 + y, 1 + z,     x, 1 + y, 1 + z,
                    });

                    this->tex.insert(this->tex.end(), {
                        0.0f + t.x, 0.0f + t.y,     0.25f + t.x, 0.0f + t.y,    0.25f + t.x, 0.25f + t.y,   0.0f + t.x, 0.0f + t.y,     0.25f + t.x, 0.25f + t.y,   0.0f + t.x, 0.25f + t.y,
                    });
                }

                // Bottom face
                if (y == 0 || (y != 0 && this->chunkData[z][y - 1][x] == 0)) {
                    const glm::vec2 t = glm::vec2(0.25f * (faces[5] % 4), floor(faces[5] / 4) / 4);

                    this->vertices.insert(this->vertices.end(), {
                        x, y, z,    x, y, 1 + z,    1 + x, y, 1 + z,    x, y, z,    1 + x, y, 1 + z,    1 + x, y, z,
                    });

                    this->tex.insert(this->tex.end(), {
                        0.0f + t.x, 0.0f + t.y,     0.25f + t.x, 0.0f + t.y,    0.25f + t.x, 0.25f + t.y,   0.0f + t.x, 0.0f + t.y,     0.25f + t.x, 0.25f + t.y,   0.0f + t.x, 0.25f + t.y,
                    });
                }

                // Front face
                if (z == CHUNK_SIZE - 1 || (z != CHUNK_SIZE - 1 && this->chunkData[z + 1][y][x] == 0)) {
                    const glm::vec2 t = glm::vec2(0.25f * (faces[0] % 4), floor(faces[0] / 4) / 4);

                    this->vertices.insert(this->vertices.end(), {
                        1 + x, y, 1 + z,    x, y, 1 + z,    x, 1 + y, 1 + z,    1 + x, y, 1 + z,    x, 1 + y, 1 + z,    1 + x, 1 + y, 1 + z,
                    });

                    this->tex.insert(this->tex.end(), {
                        0.0f + t.x, 0.0f + t.y,     0.25f + t.x, 0.0f + t.y,    0.25f + t.x, 0.25f + t.y,   0.0f + t.x, 0.0f + t.y,     0.25f + t.x, 0.25f + t.y,   0.0f + t.x, 0.25f + t.y,
                    });
                }

                // Back face
                if (z == 0 || (z != 0 && this->chunkData[z - 1][y][x] == 0)) {
                    const glm::vec2 t = glm::vec2(0.25f * (faces[1] % 4), floor(faces[1] / 4) / 4);

                    this->vertices.insert(this->vertices.end(), {
                        x, y, z,    1 + x, y, z,    1 + x, 1 + y, z,    x, y, z,    1 + x, 1 + y, z,    x, 1 + y, z,
                    });

                    this->tex.insert(this->tex.end(), {
                        0.0f + t.x, 0.0f + t.y,     0.25f + t.x, 0.0f + t.y,    0.25f + t.x, 0.25f + t.y,   0.0f + t.x, 0.0f + t.y,     0.25f + t.x, 0.25f + t.y,   0.0f + t.x, 0.25f + t.y,
                    });
                }

                // Old way
                /*this->vertices.insert(this->vertices.end(), {
                    1 + x, y, 1 + z,    x, y, 1 + z,        x, 1 + y, 1 + z,        1 + x, y, 1 + z,    x, 1 + y, 1 + z,        1 + x, 1 + y, 1 + z,             // Front face   0
                    x, y, z,            1 + x, y, z,        1 + x, 1 + y, z,        x, y, z,            1 + x, 1 + y, z,        x, 1 + y, z,                     // Back face    1
                    x, y, 1 + z,        x, y, z,            x, 1 + y, z,            x, y, 1 + z,        x, 1 + y, z,            x, 1 + y, 1 + z,                 // Left face    2
                    1 + x, y, z,        1 + x, y, 1 + z,    1 + x, 1 + y, 1 + z,    1 + x, y, z,        1 + x, 1 + y, 1 + z,    1 + x, 1 + y, z,                 // Right face   3
                    x, 1 + y, z,        1 + x, 1 + y, z,    1 + x, 1 + y, 1 + z,    x, 1 + y, z,        1+ x, 1 + y, 1 + z,     x, 1 + y, 1 + z,                 // Top face     4
                    x, y, z,            x, y, 1 + z,        1 + x, y, 1 + z,        x, y, z,            1 + x, y, 1 + z,        1 + x, y, z,                     // Bottom face  5
                });
                for (int j = 0; j < 6; j++) {
                    const glm::vec2 t = glm::vec2(0.25f * (faces[j] % 4), floor(faces[j] / 4) / 4);
                    this->tex.insert(this->tex.end(), {
                        0.0f + t.x, 0.0f + t.y,     0.25f + t.x, 0.0f + t.y,    0.25f + t.x, 0.25f + t.y,   0.0f + t.x, 0.0f + t.y,     0.25f + t.x, 0.25f + t.y,   0.0f + t.x, 0.25f + t.y,
                    });
                }*/
            }
        }
    }

    // Bind VAO
    this->VAO.bind();

    // Link datas to VA0 before rendering
    this->VAO.linkVertices(this->vertices);
    this->VAO.linkTex(this->tex);

    // Unbind VAO
    this->VAO.unbind();
}

void Chunk::bind() const {
    this->VAO.bind();
}

void Chunk::draw() const
{
    this->VAO.bind();
    glDrawArrays(GL_TRIANGLES, 0, this->vertices.size());
    this->VAO.unbind();
}
