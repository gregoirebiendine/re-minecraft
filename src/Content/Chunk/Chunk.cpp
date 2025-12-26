#include "Chunk.h"

#include "Camera.h"
#include "Camera.h"
#include "Camera.h"
#include "Camera.h"
#include "Camera.h"
#include "Camera.h"
#include "Camera.h"
#include "Camera.h"

Chunk::Chunk(const ChunkPos pos)
{
    this->_position = pos;

    // Fill the chunk with DIRT
    this->_blocks.fill(Material::DIRT);

    // Fill first layer with grass for better look
    for (uint8_t z = 0; z < SIZE; ++z)
        for (uint8_t x = 0; x < SIZE; ++x)
        {
            this->setBlock(x, 15, z, Material::GRASS);
        }

    // Iterate over all Materials to construct blocks
    // for (uint8_t z = 0; z < SIZE; z++) {
    //     for (uint8_t y = 0; y < SIZE; y++) {
    //         for (uint8_t x = 0; x < SIZE; x++) {
    //             const BlockID block = getBlock(x, y, z);
    //
    //             // Skip AIR
    //             if (this->isAir(x, y, z)) continue;
    //             // if (block == BlockID::AIR) continue;
    //
    //             // Retrieve faces atlas indexes
    //             const std::array<uint8_t, 6> blockTexFaces = texAtlas[block];
    //             std::vector<BlockFaces> renderedFaces;
    //
    //             // Front face (0)
    //             if (z == SIZE - 1 || (z != SIZE - 1 && this->isAir(x, y, z + 1))) {
    //                 this->vertices.insert(this->vertices.end(), {
    //                     1 + x + position.x, y + position.y, 1 + z + position.z,
    //                     x + position.x, y + position.y, 1 + z + position.z,
    //                     x + position.x, 1 + y + position.y, 1 + z + position.z,
    //                     1 + x + position.x, y + position.y, 1 + z + position.z,
    //                     x + position.x, 1 + y + position.y, 1 + z + position.z,
    //                     1 + x + position.x, 1 + y + position.y, 1 + z + position.z,
    //                 });
    //                 renderedFaces.push_back(BlockFaces::FRONT);
    //             }
    //
    //             // Back face (1)
    //             if (z == 0 || (z != 0 && this->isAir(x, y, z - 1))) {
    //                 this->vertices.insert(this->vertices.end(), {
    //                     x + position.x, y + position.y, z + position.z,
    //                     1 + x + position.x, y + position.y, z + position.z,
    //                     1 + x + position.x, 1 + y + position.y, z + position.z,
    //                     x + position.x, y + position.y, z + position.z,
    //                     1 + x + position.x, 1 + y + position.y, z + position.z,
    //                     x + position.x, 1 + y + position.y, z + position.z,
    //                 });
    //                 renderedFaces.push_back(BlockFaces::BACK);
    //             }
    //
    //             // Left face (2)
    //             if (x == 0 || (x != 0 && this->isAir(x - 1, y, z))) {
    //                 this->vertices.insert(this->vertices.end(), {
    //                     x + position.x, y + position.y, 1 + z + position.z,
    //                     x + position.x, y + position.y, z + position.z,
    //                     x + position.x, 1 + y + position.y, z + position.z,
    //                     x + position.x, y + position.y, 1 + z + position.z,
    //                     x + position.x, 1 + y + position.y, z + position.z,
    //                     x + position.x, 1 + y + position.y, 1 + z + position.z,
    //                 });
    //                 renderedFaces.push_back(BlockFaces::LEFT);
    //             }
    //
    //             // Right face (3)
    //             if (x == SIZE - 1 || (x != SIZE - 1 && this->isAir(x + 1, y, z))) {
    //                 this->vertices.insert(this->vertices.end(), {
    //                     1 + x + position.x, y + position.y, z + position.z,
    //                     1 + x + position.x, y + position.y, 1 + z + position.z,
    //                     1 + x + position.x, 1 + y + position.y, 1 + z + position.z,
    //                     1 + x + position.x, y + position.y, z + position.z,
    //                     1 + x + position.x, 1 + y + position.y, 1 + z + position.z,
    //                     1 + x + position.x, 1 + y + position.y, z + position.z,
    //                 });
    //                 renderedFaces.push_back(BlockFaces::RIGHT);
    //             }
    //
    //             // Top face (4)
    //             if (y == SIZE - 1 || (y != SIZE - 1 && this->isAir(x, y + 1, z))) {
    //                 this->vertices.insert(this->vertices.end(), {
    //                     x + position.x, 1 + y + position.y, z + position.z,
    //                     1 + x + position.x, 1 + y + position.y, z + position.z,
    //                     1 + x + position.x, 1 + y + position.y, 1 + z + position.z,
    //                     x + position.x, 1 + y + position.y, z + position.z,
    //                     1 + x + position.x, 1 + y + position.y, 1 + z + position.z,
    //                     x + position.x, 1 + y + position.y, 1 + z + position.z,
    //                 });
    //                 renderedFaces.push_back(BlockFaces::TOP);
    //             }
    //
    //             // Bottom face (5)
    //             if (y == 0 || (y != 0 && this->isAir(x, y - 1, z))) {
    //                 this->vertices.insert(this->vertices.end(), {
    //                     x + position.x, y + position.y, z + position.z,
    //                     x + position.x, y + position.y, 1 + z + position.z,
    //                     1 + x + position.x, y + position.y, 1 + z + position.z,
    //                     x + position.x, y + position.y, z + position.z,
    //                     1 + x + position.x, y + position.y, 1 + z + position.z,
    //                     1 + x + position.x, y + position.y, z + position.z,
    //                 });
    //                 renderedFaces.push_back(BlockFaces::BOTTOM);
    //             }
    //
    //             // Add tex offset based on rendered faces
    //             for (const auto &face : renderedFaces) {
    //                 const auto t = glm::vec2(0.25f * static_cast<float>(blockTexFaces[face] % 4), floor(static_cast<double>(blockTexFaces[face]) / 4) / 4);
    //                 this->tex.insert(this->tex.end(), {
    //                     t.x, t.y,     0.25f + t.x, t.y,    0.25f + t.x, 0.25f + t.y,   t.x, t.y,     0.25f + t.x, 0.25f + t.y,   t.x, 0.25f + t.y,
    //                 });
    //             }
    //         }
    //     }
    // }
    //
    // // Bind VAO
    // this->VAO.bind();
    //
    // // Link datas to VA0 before rendering
    // this->VAO.linkVertices(this->vertices);
    // this->VAO.linkTex(this->tex);
    //
    // // Unbind VAO
    // this->VAO.unbind();
}

uint16_t Chunk::index(const uint8_t x, const uint8_t y, const uint8_t z)
{
    return clamp(x) + SIZE * (clamp(y) + SIZE * clamp(z));
}

uint8_t Chunk::clamp(const uint8_t v)
{
    return std::clamp(v, static_cast<uint8_t>(0), SIZE);
}

glm::mat<4, 4, float> Chunk::getChunkModel() const
{
    const auto offset = glm::vec3(
        this->_position.x * Chunk::SIZE,
        this->_position.y * Chunk::SIZE,
        this->_position.z * Chunk::SIZE
    );
    return glm::translate(glm::mat4(1.0f), offset);
}

ChunkPos Chunk::getPosition() const
{
    return this->_position;
}

Material Chunk::getBlock(const uint8_t x, const uint8_t y, const uint8_t z) const
{
    return _blocks[index(x, y, z)];
}

bool Chunk::isDirty() const
{
    return this->_isDirty;
}

bool Chunk::isAir(const uint8_t x, const uint8_t y, const uint8_t z) const
{
    return _blocks[index(x, y, z)] == Material::AIR;
}


void Chunk::setBlock(const uint8_t x, const uint8_t y, const uint8_t z, const Material id)
{
    _blocks[index(x, y, z)] = id;
}

void Chunk::setDirty(const bool dirty)
{
    this->_isDirty = dirty;
}
