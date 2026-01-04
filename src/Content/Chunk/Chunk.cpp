#include "Chunk.h"

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

bool Chunk::isAir(const uint8_t x, const uint8_t y, const uint8_t z) const
{
    return _blocks[index(x, y, z)] == Material::AIR;
}

bool Chunk::isDirty() const
{
    return this->_isDirty;
}

void Chunk::setBlock(const uint8_t x, const uint8_t y, const uint8_t z, const Material id)
{
    this->_blocks[index(x, y, z)] = id;
    this->setDirty(true);
}

void Chunk::setDirty(const bool dirty)
{
    this->_isDirty = dirty;
}

// Statics
uint16_t Chunk::index(const uint8_t x, const uint8_t y, const uint8_t z)
{
    return clamp(x) + SIZE * (clamp(y) + SIZE * clamp(z));
}

uint8_t Chunk::clamp(const uint8_t v)
{
    return std::clamp(v, static_cast<uint8_t>(0), SIZE);
}