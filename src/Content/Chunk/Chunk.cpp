#include "Chunk.h"

Chunk::Chunk(const ChunkPos pos) :
    _position(pos)
{
    // Empty
}

glm::mat<4, 4, float> Chunk::getChunkModel() const
{
    const auto offset = glm::vec3(
        this->_position.x * SIZE,
        this->_position.y * SIZE,
        this->_position.z * SIZE
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
    return _blocks[index(x, y, z)] == 0;
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

void Chunk::fill(const glm::ivec3 from, const glm::ivec3 to, const Material id)
{
    for (int z = from.z; z <= to.z; ++z)
        for (int y = from.y; y <= to.y; ++y)
            for (int x = from.x; x <= to.x; ++x)
            {
                this->setBlock(x, y, z, id);
            }
}

void Chunk::setDirty(const bool dirty)
{
    this->_isDirty = dirty;
}

const ChunkState& Chunk::getState() const
{
    return this->_state;
}

void Chunk::setState(const ChunkState state)
{
    this->_state = state;
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