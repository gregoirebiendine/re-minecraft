#include "Chunk.h"

constexpr uint8_t CHUNK_SIZE = 16;
constexpr uint16_t MAX_BLOCK = CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE;

std::map<uint8_t, std::array<uint8_t, 6>> blockFaces = {
    {0, {0, 0, 0, 0, 0, 0}},    // Air
    {1, {0, 0, 0, 0, 0, 0}},    // Dirt
    {2, {1, 1, 1, 1, 2, 0}},    // Grass
    {3, {2, 2, 2, 2, 2, 2}},    // Moss
};

enum BlockFaces {
    FRONT,
    BACK,
    LEFT,
    RIGHT,
    TOP,
    BOTTOM
};

Chunk::Chunk(glm::ivec3 offset)
{
    this->chunkOffset = offset;

    // Fill the chunk with Materials (dirt for instance)
    for (int i = 0; i < MAX_BLOCK; i++) {
        const auto coords = glm::uvec3(i / (CHUNK_SIZE * CHUNK_SIZE), (i / CHUNK_SIZE) % CHUNK_SIZE, i % CHUNK_SIZE);
        this->chunkData[coords.z][coords.y][coords.x] = 1;
    }

    // Fill first layer with grass for better look
    // for (auto &z : this->chunkData) {
    //     for (uint8_t x = 0; x < CHUNK_SIZE; x++) {
    //         z[15][x] = 2;
    //     }
    // }

    // Add air in top of the chunk to test robustness of the code
    // this->chunkData[8][15][8] = 0;

    // Iterate over all Materials to construct blocks
    for (uint8_t z = 0; z < CHUNK_SIZE; z++) {
        for (uint8_t y = 0; y < CHUNK_SIZE; y++) {
            for (uint8_t x = 0; x < CHUNK_SIZE; x++) {
                // Skip AIR (index 0)
                if (chunkData[z][y][x] == 0) continue;

                // Retrieve faces atlas indexes
                const std::array<uint8_t, 6> faces = blockFaces[chunkData[z][y][x]];
                std::vector<BlockFaces> renderedFaces;

                // Front face (0)
                if (z == CHUNK_SIZE - 1 || (z != CHUNK_SIZE - 1 && this->chunkData[z + 1][y][x] == 0)) {
                    this->vertices.insert(this->vertices.end(), {
                        1 + x + chunkOffset.x, y + chunkOffset.y, 1 + z + chunkOffset.z,
                        x + chunkOffset.x, y + chunkOffset.y, 1 + z + chunkOffset.z,
                        x + chunkOffset.x, 1 - y + chunkOffset.y, 1 + z + chunkOffset.z,
                        1 + x + chunkOffset.x, y + chunkOffset.y, 1 + z + chunkOffset.z,
                        x + chunkOffset.x, 1 - y + chunkOffset.y, 1 + z + chunkOffset.z,
                        1 + x + chunkOffset.x, 1 - y + chunkOffset.y, 1 + z + chunkOffset.z,
                    });
                    renderedFaces.push_back(BlockFaces::FRONT);
                }

                // Back face (1)
                if (z == 0 || (z != 0 && this->chunkData[z - 1][y][x] == 0)) {
                    this->vertices.insert(this->vertices.end(), {
                        x + chunkOffset.x, y + chunkOffset.y, z + chunkOffset.z,
                        1 + x + chunkOffset.x, y + chunkOffset.y, z + chunkOffset.z,
                        1 + x + chunkOffset.x, 1 - y + chunkOffset.y, z + chunkOffset.z,
                        x + chunkOffset.x, y + chunkOffset.y, z + chunkOffset.z,
                        1 + x + chunkOffset.x, 1 - y + chunkOffset.y, z + chunkOffset.z,
                        x + chunkOffset.x, 1 - y + chunkOffset.y, z + chunkOffset.z,
                    });
                    renderedFaces.push_back(BlockFaces::BACK);
                }

                // Left face (2)
                if (x == 0 || (x != 0 && this->chunkData[z][y][x - 1] == 0)) {
                    this->vertices.insert(this->vertices.end(), {
                        x + chunkOffset.x, y + chunkOffset.y, 1 + z + chunkOffset.z,
                        x + chunkOffset.x, y + chunkOffset.y, z + chunkOffset.z,
                        x + chunkOffset.x, 1 - y + chunkOffset.y, z + chunkOffset.z,
                        x + chunkOffset.x, y + chunkOffset.y, 1 + z + chunkOffset.z,
                        x + chunkOffset.x, 1 - y + chunkOffset.y, z + chunkOffset.z,
                        x + chunkOffset.x, 1 - y + chunkOffset.y, 1 + z + chunkOffset.z,
                    });
                    renderedFaces.push_back(BlockFaces::LEFT);
                }

                // Right face (3)
                if (x == CHUNK_SIZE - 1 || (x != CHUNK_SIZE - 1 && this->chunkData[z][y][x + 1] == 0)) {
                    this->vertices.insert(this->vertices.end(), {
                        1 + x + chunkOffset.x, y + chunkOffset.y, z + chunkOffset.z,
                        1 + x + chunkOffset.x, y + chunkOffset.y, 1 + z + chunkOffset.z,
                        1 + x + chunkOffset.x, 1 - y + chunkOffset.y, 1 + z + chunkOffset.z,
                        1 + x + chunkOffset.x, y + chunkOffset.y, z + chunkOffset.z,
                        1 + x + chunkOffset.x, 1 - y + chunkOffset.y, 1 + z + chunkOffset.z,
                        1 + x + chunkOffset.x, 1 - y + chunkOffset.y, z + chunkOffset.z,
                    });
                    renderedFaces.push_back(BlockFaces::RIGHT);
                }

                // Top face (4)
                if (y == CHUNK_SIZE - 1 || (y != CHUNK_SIZE - 1 && this->chunkData[z][y - 1][x] == 0)) {
                    this->vertices.insert(this->vertices.end(), {
                        x + chunkOffset.x, 1 - y + chunkOffset.y, z + chunkOffset.z,
                        1 + x + chunkOffset.x, 1 - y + chunkOffset.y, z + chunkOffset.z,
                        1 + x + chunkOffset.x, 1 - y + chunkOffset.y, 1 + z + chunkOffset.z,
                        x + chunkOffset.x, 1 - y + chunkOffset.y, z + chunkOffset.z,
                        1 + x + chunkOffset.x, 1 - y + chunkOffset.y, 1 + z + chunkOffset.z,
                        x + chunkOffset.x, 1 - y + chunkOffset.y, 1 + z + chunkOffset.z,
                    });
                    renderedFaces.push_back(BlockFaces::TOP);
                }

                // Bottom face (5)
                if (y == 0 || (y != 0 && this->chunkData[z][y - 1][x] == 0)) {
                    this->vertices.insert(this->vertices.end(), {
                        x + chunkOffset.x, y + chunkOffset.y, z + chunkOffset.z,
                        x + chunkOffset.x, y + chunkOffset.y, 1 + z + chunkOffset.z,
                        1 + x + chunkOffset.x, y + chunkOffset.y, 1 + z + chunkOffset.z,
                        x + chunkOffset.x, y + chunkOffset.y, z + chunkOffset.z,
                        1 + x + chunkOffset.x, y + chunkOffset.y, 1 + z + chunkOffset.z,
                        1 + x + chunkOffset.x, y + chunkOffset.y, z + chunkOffset.z,
                    });
                    renderedFaces.push_back(BlockFaces::BOTTOM);
                }

                // Add tex offset based of rendered faces
                for (const auto &face : renderedFaces) {
                    const auto t = glm::vec2(0.25f * (faces[face] % 4), floor(faces[face] / 4) / 4);
                    this->tex.insert(this->tex.end(), {
                        t.x, t.y,     0.25f + t.x, t.y,    0.25f + t.x, 0.25f + t.y,   t.x, t.y,     0.25f + t.x, 0.25f + t.y,   t.x, 0.25f + t.y,
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
