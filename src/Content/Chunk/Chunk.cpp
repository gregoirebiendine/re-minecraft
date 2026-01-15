#include "Chunk.h"

Chunk::Chunk(const ChunkPos pos) :
    position(pos)
{
    // Empty
}

glm::mat<4, 4, float> Chunk::getChunkModel() const
{
    const auto offset = glm::vec3(
        this->position.x * SIZE,
        this->position.y * SIZE,
        this->position.z * SIZE
    );
    return glm::translate(glm::mat4(1.0f), offset);
}

ChunkPos Chunk::getPosition() const
{
    return this->position;
}

Material Chunk::getBlock(const uint8_t x, const uint8_t y, const uint8_t z) const
{
    return this->blocks[ChunkCoords::localCoordsToIndex(x, y, z)];
}

bool Chunk::isAir(const uint8_t x, const uint8_t y, const uint8_t z) const
{
    return this->blocks[ChunkCoords::localCoordsToIndex(x, y, z)] == 0;
}

void Chunk::setBlock(const uint8_t x, const uint8_t y, const uint8_t z, const Material id)
{
    this->blocks[ChunkCoords::localCoordsToIndex(x, y, z)] = id;
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

ChunkState Chunk::getState() const
{
    return this->state.load();
}

void Chunk::setState(const ChunkState _state)
{
    this->state.store(_state);
}

uint64_t Chunk::getGenerationID() const
{
    return generationID.load();
}

void Chunk::bumpGenerationID()
{
    generationID.fetch_add(1);
}

// Statics
// uint16_t Chunk::index(const uint8_t x, const uint8_t y, const uint8_t z)
// {
//     return clamp(x) + SIZE * (clamp(y) + SIZE * clamp(z));
// }
//
// uint8_t Chunk::clamp(const uint8_t v)
// {
//     return std::clamp(v, static_cast<uint8_t>(0), SIZE);
// }