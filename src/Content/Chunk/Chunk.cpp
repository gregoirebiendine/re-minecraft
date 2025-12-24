#include "Chunk.h"

enum BlockID : uint8_t
{
    AIR,
    DIRT,
    GRASS,
    MOSS,
};

enum BlockFaces
{
    FRONT,
    BACK,
    LEFT,
    RIGHT,
    TOP,
    BOTTOM
};

std::map<BlockID, std::array<uint8_t, 6>> texAtlas = {
    {AIR, {0, 0, 0, 0, 0, 0}},
    {DIRT, {0, 0, 0, 0, 0, 0}},
    {GRASS, {1, 1, 1, 1, 2, 0}},
    {MOSS, {2, 2, 2, 2, 2, 2}},
};

Chunk::Chunk(const glm::uvec3 offset)
{
    this->_offset = offset;

    // Fill the chunk with DIRT
    this->blocks.fill(BlockID::DIRT);

    // Fill first layer with grass for better look
    for (uint8_t z = 0; z < SIZE; ++z)
        for (uint8_t x = 0; x < SIZE; ++x)
        {
            this->setBlock(x, 15, z, BlockID::GRASS);
        }


    // Iterate over all Materials to construct blocks
    for (uint8_t z = 0; z < SIZE; z++) {
        for (uint8_t y = 0; y < SIZE; y++) {
            for (uint8_t x = 0; x < SIZE; x++) {
                const BlockID block = getBlock(x, y, z);

                // Skip AIR
                if (block == BlockID::AIR) continue;

                // Retrieve faces atlas indexes
                const std::array<uint8_t, 6> blockTexFaces = texAtlas[block];
                std::vector<BlockFaces> renderedFaces;

                // Front face (0)
                if (z == SIZE - 1 || (z != SIZE - 1 && getBlock(x, y, z + 1) == AIR)) {
                    this->vertices.insert(this->vertices.end(), {
                        1 + x + _offset.x, y + _offset.y, 1 + z + _offset.z,
                        x + _offset.x, y + _offset.y, 1 + z + _offset.z,
                        x + _offset.x, 1 + y + _offset.y, 1 + z + _offset.z,
                        1 + x + _offset.x, y + _offset.y, 1 + z + _offset.z,
                        x + _offset.x, 1 + y + _offset.y, 1 + z + _offset.z,
                        1 + x + _offset.x, 1 + y + _offset.y, 1 + z + _offset.z,
                    });
                    renderedFaces.push_back(BlockFaces::FRONT);
                }

                // Back face (1)
                if (z == 0 || (z != 0 && getBlock(x, y, z - 1) == AIR)) {
                    this->vertices.insert(this->vertices.end(), {
                        x + _offset.x, y + _offset.y, z + _offset.z,
                        1 + x + _offset.x, y + _offset.y, z + _offset.z,
                        1 + x + _offset.x, 1 + y + _offset.y, z + _offset.z,
                        x + _offset.x, y + _offset.y, z + _offset.z,
                        1 + x + _offset.x, 1 + y + _offset.y, z + _offset.z,
                        x + _offset.x, 1 + y + _offset.y, z + _offset.z,
                    });
                    renderedFaces.push_back(BlockFaces::BACK);
                }

                // Left face (2)
                if (x == 0 || (x != 0 && getBlock(x - 1, y, z) == AIR)) {
                    this->vertices.insert(this->vertices.end(), {
                        x + _offset.x, y + _offset.y, 1 + z + _offset.z,
                        x + _offset.x, y + _offset.y, z + _offset.z,
                        x + _offset.x, 1 + y + _offset.y, z + _offset.z,
                        x + _offset.x, y + _offset.y, 1 + z + _offset.z,
                        x + _offset.x, 1 + y + _offset.y, z + _offset.z,
                        x + _offset.x, 1 + y + _offset.y, 1 + z + _offset.z,
                    });
                    renderedFaces.push_back(BlockFaces::LEFT);
                }

                // Right face (3)
                if (x == SIZE - 1 || (x != SIZE - 1 && getBlock(x + 1, y, z) == AIR)) {
                    this->vertices.insert(this->vertices.end(), {
                        1 + x + _offset.x, y + _offset.y, z + _offset.z,
                        1 + x + _offset.x, y + _offset.y, 1 + z + _offset.z,
                        1 + x + _offset.x, 1 + y + _offset.y, 1 + z + _offset.z,
                        1 + x + _offset.x, y + _offset.y, z + _offset.z,
                        1 + x + _offset.x, 1 + y + _offset.y, 1 + z + _offset.z,
                        1 + x + _offset.x, 1 + y + _offset.y, z + _offset.z,
                    });
                    renderedFaces.push_back(BlockFaces::RIGHT);
                }

                // Top face (4)
                if (y == SIZE - 1 || (y != SIZE - 1 && getBlock(x, y + 1, z) == AIR)) {
                    this->vertices.insert(this->vertices.end(), {
                        x + _offset.x, 1 + y + _offset.y, z + _offset.z,
                        1 + x + _offset.x, 1 + y + _offset.y, z + _offset.z,
                        1 + x + _offset.x, 1 + y + _offset.y, 1 + z + _offset.z,
                        x + _offset.x, 1 + y + _offset.y, z + _offset.z,
                        1 + x + _offset.x, 1 + y + _offset.y, 1 + z + _offset.z,
                        x + _offset.x, 1 + y + _offset.y, 1 + z + _offset.z,
                    });
                    renderedFaces.push_back(BlockFaces::TOP);
                }

                // Bottom face (5)
                if (y == 0 || (y != 0 && getBlock(x, y - 1, z) == AIR)) {
                    this->vertices.insert(this->vertices.end(), {
                        x + _offset.x, y + _offset.y, z + _offset.z,
                        x + _offset.x, y + _offset.y, 1 + z + _offset.z,
                        1 + x + _offset.x, y + _offset.y, 1 + z + _offset.z,
                        x + _offset.x, y + _offset.y, z + _offset.z,
                        1 + x + _offset.x, y + _offset.y, 1 + z + _offset.z,
                        1 + x + _offset.x, y + _offset.y, z + _offset.z,
                    });
                    renderedFaces.push_back(BlockFaces::BOTTOM);
                }

                // Add tex offset based on rendered faces
                for (const auto &face : renderedFaces) {
                    const auto t = glm::vec2(0.25f * static_cast<float>(blockTexFaces[face] % 4), floor(static_cast<double>(blockTexFaces[face]) / 4) / 4);
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

uint16_t Chunk::index(const uint8_t x, const uint8_t y, const uint8_t z)
{
    return clamp(x) + SIZE * (clamp(y) + SIZE * clamp(z));
}

uint8_t Chunk::clamp(const uint8_t v)
{
    return std::clamp(v, static_cast<uint8_t>(0), SIZE);
}

BlockID Chunk::getBlock(const uint8_t x, const uint8_t y, const uint8_t z) const
{
    return blocks[index(x, y, z)];
}

void Chunk::setBlock(const uint8_t x, const uint8_t y, const uint8_t z, const BlockID id)
{
    blocks[index(x, y, z)] = id;
}

void Chunk::bind() const {
    this->VAO.bind();
}

void Chunk::render() const
{
    this->VAO.bind();
    glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(this->vertices.size()));
    this->VAO.unbind();
}
