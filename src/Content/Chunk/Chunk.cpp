#include "Chunk.h"

constexpr uint8_t CHUNK_SIZE = 2;
constexpr uint16_t MAX_BLOCK = CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE;

std::map<uint8_t, std::array<uint8_t, 6>> blockFaces = {
    {0, {0, 0, 0, 0, 0, 0}},
    {1, {1, 1, 1, 1, 2, 0}},
    {2, {2, 2, 2, 2, 2, 2}},
};

Chunk::Chunk()
{
    // Fill the chunk with Materials
    for (int i = 0; i < MAX_BLOCK; i++) {
        const glm::uvec3 coords = glm::uvec3(i / (CHUNK_SIZE * CHUNK_SIZE), (i / CHUNK_SIZE) % CHUNK_SIZE, i % CHUNK_SIZE);
        this->chunkData[coords.z][coords.y][coords.x] = 0;
    }

    // Iterate over all Materials to construct blocks
    for (int z = 0; z < CHUNK_SIZE; z++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int x = 0; x < CHUNK_SIZE; x++) {
                const std::array<uint8_t, 6> faces = blockFaces[chunkData[z][y][x]];

                this->vertices.insert(this->vertices.end(), {
                    1 + x, y, 1 + z,    x, y, 1 + z,        x, 1 + y, 1 + z,        1 + x, y, 1 + z,    x, 1 + y, 1 + z,        1 + x, 1 + y, 1 + z,                // Front face
                    x, y, z,            1 + x, y, z,        1 + x, 1 + y, z,        x, y, z,            1 + x, 1 + y, z,        x, 1 + y, z,                        // Back face
                    x, y, 1 + z,        x, y, z,            x, 1 + y, z,            x, y, 1 + z,        x, 1 + y, z,            x, 1 + y, 1 + z,                    // Left face
                    1 + x, y, z,        1 + x, y, 1 + z,    1 + x, 1 + y, 1 + z,    1 + x, y, z,        1 + x, 1 + y, 1 + z,    1 + x, 1 + y, z,                    // Right face
                    x, 1 + y, z,        1 + x, 1 + y, z,    1 + x, 1 + y, 1 + z,    x, 1 + y, z,        1+ x, 1 + y, 1 + z,     x, 1 + y, 1 + z,                    // Top face
                    x, y, z,            x, y, 1 + z,        1 + x, y, 1 + z,        x, y, z,            1 + x, y, 1 + z,        1 + x, y, z,                        // Bottom face
                });

                for (int j = 0; j < 6; j++) {
                    const glm::vec2 t = glm::vec2(0.25f * (faces[j] % 4), floor(faces[j] / 4) / 4);
                    this->tex.insert(this->tex.end(), {
                        0.0f + t.x, 0.0f + t.y,     0.25f + t.x, 0.0f + t.y,    0.25f + t.x, 0.25f + t.y,   0.0f + t.x, 0.0f + t.y,     0.25f + t.x, 0.25f + t.y,   0.0f + t.x, 0.25f + t.y,
                    });
                }
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
