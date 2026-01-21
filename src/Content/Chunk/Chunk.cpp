#include "Chunk.h"

Chunk::Chunk(const ChunkPos pos) :
    position(pos)
{
    this->blockBuffers[0].fill(0);
    this->blockBuffers[1].fill(0);
}

Chunk::Chunk(Chunk&& other) noexcept :
    position(other.position),
    blockBuffers{other.blockBuffers[0], other.blockBuffers[1]},
    bufferReadIndex(other.bufferReadIndex.load()),
    bufferActiveReaders(other.bufferActiveReaders.load()),
    pendingChanges(other.pendingChanges.load()),
    state(other.state.load()),
    generationID(other.generationID.load()),
    dirty(other.dirty.load())
{
    // Empty
}

Chunk& Chunk::operator=(Chunk&& other) noexcept
{
    if (this != &other) {
        position = other.position;
        blockBuffers[0] = other.blockBuffers[0];
        blockBuffers[1] = other.blockBuffers[1];
        bufferReadIndex.store(other.bufferReadIndex.load());
        bufferActiveReaders.store(other.bufferActiveReaders.load());
        pendingChanges.store(other.pendingChanges.load());
        state.store(other.state.load());
        generationID.store(other.generationID.load());
        dirty.store(other.dirty.load());
    }
    return *this;
}



uint8_t Chunk::acquireRead() const
{
    bufferActiveReaders.fetch_add(1, std::memory_order_acquire);
    return bufferReadIndex.load(std::memory_order_acquire);
}

void Chunk::releaseRead() const
{
    bufferActiveReaders.fetch_sub(1, std::memory_order_release);
}

std::array<Material, Chunk::VOLUME> Chunk::getBlockSnapshot() const
{
    // Acquire read access
    this->acquireRead();

    // Copy the entire front buffer
    const uint8_t readIdx = bufferReadIndex.load(std::memory_order_acquire);
    const std::array<Material, VOLUME> snapshot = blockBuffers[readIdx];

    // Release read access
    this->releaseRead();

    return snapshot;
}



void Chunk::setBlock(const uint8_t x, const uint8_t y, const uint8_t z, const Material id)
{
    const uint8_t writeIdx = getWriteIndex();
    blockBuffers[writeIdx][ChunkCoords::localCoordsToIndex(x, y, z)] = id;
    pendingChanges.store(true, std::memory_order_release);
}

void Chunk::fill(const glm::ivec3 from, const glm::ivec3 to, const Material id)
{
    const uint8_t writeIdx = getWriteIndex();

    for (int z = from.z; z <= to.z; ++z) {
        for (int y = from.y; y <= to.y; ++y) {
            for (int x = from.x; x <= to.x; ++x) {
                this->blockBuffers[writeIdx][ChunkCoords::localCoordsToIndex(x, y, z)] = id;
            }
        }
    }

    this->pendingChanges.store(true, std::memory_order_release);
}

bool Chunk::swapBuffers()
{
    if (!pendingChanges.load(std::memory_order_acquire))
        return false;

    while (bufferActiveReaders.load(std::memory_order_acquire) > 0) {
        std::this_thread::yield();
    }

    std::atomic_thread_fence(std::memory_order_seq_cst);

    const uint8_t oldReadIdx = this->bufferReadIndex.load(std::memory_order_acquire);
    const uint8_t newReadIdx = 1 - oldReadIdx;

    this->blockBuffers[oldReadIdx] = this->blockBuffers[newReadIdx];
    this->bufferReadIndex.store(newReadIdx, std::memory_order_release);
    this->pendingChanges.store(false, std::memory_order_release);

    return true;
}

bool Chunk::hasPendingChanges() const
{
    return pendingChanges.load(std::memory_order_acquire);
}



void Chunk::setBlockDirect(uint8_t x, uint8_t y, uint8_t z, Material id)
{
    this->blockBuffers[0][ChunkCoords::localCoordsToIndex(x, y, z)] = id;
}

void Chunk::finalizeGeneration()
{
    this->blockBuffers[1] = this->blockBuffers[0];
    this->bufferReadIndex.store(0, std::memory_order_release);
    this->pendingChanges.store(false, std::memory_order_release);
}



Material Chunk::getBlock(uint8_t x, uint8_t y, uint8_t z) const
{
    const uint8_t readIdx = bufferReadIndex.load(std::memory_order_acquire);
    return blockBuffers[readIdx][ChunkCoords::localCoordsToIndex(x, y, z)];
}

bool Chunk::isAir(uint8_t x, uint8_t y, uint8_t z) const
{
    const uint8_t readIdx = bufferReadIndex.load(std::memory_order_acquire);
    return blockBuffers[readIdx][ChunkCoords::localCoordsToIndex(x, y, z)] == 0;
}



glm::mat4 Chunk::getChunkModel() const
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

ChunkState Chunk::getState() const
{
    return this->state.load(std::memory_order_acquire);
}

void Chunk::setState(const ChunkState newState)
{
    this->state.store(newState, std::memory_order_release);
}

bool Chunk::isDirty() const
{
    return this->dirty.load(std::memory_order_acquire);
}

void Chunk::setDirty(bool dirty)
{
    this->dirty.store(dirty, std::memory_order_release);
}

uint64_t Chunk::getGenerationID() const
{
    return this->generationID.load(std::memory_order_acquire);
}

void Chunk::bumpGenerationID()
{
    this->generationID.fetch_add(1, std::memory_order_acq_rel);
}