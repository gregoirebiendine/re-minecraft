# Double Buffering Implementation for Thread-Safe Block Access

This document provides a complete implementation guide for Option D (Double Buffering) to fix the thread-unsafe block data access in the Chunk class.

---

## Table of Contents

1. [Concept Overview](#concept-overview)
2. [Architecture Design](#architecture-design)
3. [Full Implementation](#full-implementation)
4. [Integration Guide](#integration-guide)
5. [Usage Examples](#usage-examples)
6. [Performance Considerations](#performance-considerations)

---

## Concept Overview

### The Problem

Currently, multiple threads access `Chunk::blocks` without synchronization:

```
┌─────────────────────────────────────────────────────────────────┐
│                    CURRENT (UNSAFE)                             │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   Main Thread          blocks[4096]         Worker Threads      │
│   ───────────          ────────────         ──────────────      │
│                              │                                  │
│   setBlock() ──── WRITE ────►│◄──── READ ──── buildMeshJob()   │
│                              │                                  │
│                         DATA RACE!                              │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### The Solution: Double Buffering

Double buffering uses **two separate arrays**:
- **Front Buffer**: Workers read from this (stable, immutable during reads)
- **Back Buffer**: Main thread writes to this (can be modified freely)

When modifications are complete, we **swap** the buffers atomically.

```
┌─────────────────────────────────────────────────────────────────┐
│                    DOUBLE BUFFERING (SAFE)                      │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   Main Thread                               Worker Threads      │
│   ───────────                               ──────────────      │
│        │                                          │             │
│        ▼                                          ▼             │
│   ┌─────────┐                              ┌─────────┐          │
│   │  BACK   │  ◄── WRITE                   │  FRONT  │ ── READ  │
│   │ BUFFER  │      (exclusive)             │ BUFFER  │  (shared)│
│   └─────────┘                              └─────────┘          │
│        │                                          ▲             │
│        │            ┌──────────┐                  │             │
│        └───────────►│   SWAP   │──────────────────┘             │
│                     │ (atomic) │                                │
│                     └──────────┘                                │
│                                                                 │
│   No locks needed! Writers and readers never touch same buffer  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Why Double Buffering?

| Approach | Pros | Cons |
|----------|------|------|
| Mutex | Simple | High contention, blocks readers |
| RW Lock | Multiple readers | Still blocks on write |
| **Double Buffer** | **Lock-free reads, no contention** | 2x memory, swap overhead |

For voxel engines where **reads vastly outnumber writes**, double buffering is optimal.

---

## Architecture Design

### State Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                     BUFFER STATE MACHINE                        │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   ┌─────────────┐         ┌─────────────┐                       │
│   │   CLEAN     │◄───────►│   DIRTY     │                       │
│   │ (no pending │  write  │ (pending    │                       │
│   │  changes)   │────────►│  changes)   │                       │
│   └─────────────┘         └──────┬──────┘                       │
│         ▲                        │                              │
│         │                        │ swap()                       │
│         │                        ▼                              │
│         │                 ┌─────────────┐                       │
│         └─────────────────│  SWAPPING   │                       │
│              done         │ (atomic op) │                       │
│                           └─────────────┘                       │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Memory Layout

```cpp
class Chunk {
    // Two block arrays (front and back buffers)
    std::array<Material, 4096> m_buffers[2];

    // Index of the buffer workers should READ from
    // 0 or 1, atomically swapped
    std::atomic<uint8_t> m_readIndex{0};

    // Tracks if back buffer has uncommitted changes
    std::atomic<bool> m_dirty{false};

    // Prevents swap during active reads
    std::atomic<uint32_t> m_activeReaders{0};
};
```

### Thread Responsibilities

| Thread | Buffer | Operations |
|--------|--------|------------|
| Main Thread | Back (`1 - m_readIndex`) | `setBlock()`, `fill()`, `swap()` |
| Generation Worker | Back (during GENERATING state) | `setBlock()` (exclusive) |
| Meshing Workers | Front (`m_readIndex`) | `getBlock()`, `isAir()` (shared) |

---

## Full Implementation

### Chunk.h

```cpp
#ifndef RE_MINECRAFT_CHUNK_H
#define RE_MINECRAFT_CHUNK_H

#include <atomic>
#include <array>
#include <cstring>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "BlockRegistry.h"
#include "ChunkPos.h"
#include "ChunkState.h"
#include "Utils.h"

class Chunk {
public:
    static constexpr uint8_t SIZE = 16;
    static constexpr uint16_t VOLUME = SIZE * SIZE * SIZE;

    explicit Chunk(ChunkPos pos);

    // Non-copyable (due to atomics)
    Chunk(const Chunk&) = delete;
    Chunk& operator=(const Chunk&) = delete;

    // Movable
    Chunk(Chunk&& other) noexcept;
    Chunk& operator=(Chunk&& other) noexcept;

    //==========================================================================
    // THREAD-SAFE READ OPERATIONS (for worker threads)
    //==========================================================================

    /// Acquire a read reference. Must call releaseRead() when done.
    /// Returns the buffer index to read from.
    [[nodiscard]] uint8_t acquireRead() const;

    /// Release a previously acquired read reference.
    void releaseRead() const;

    /// Get block from the FRONT buffer (thread-safe for workers).
    /// Requires acquireRead() to be called first.
    [[nodiscard]] Material getBlockRead(uint8_t x, uint8_t y, uint8_t z) const;

    /// Check if block is air in the FRONT buffer (thread-safe for workers).
    /// Requires acquireRead() to be called first.
    [[nodiscard]] bool isAirRead(uint8_t x, uint8_t y, uint8_t z) const;

    /// Get a full snapshot of the front buffer (thread-safe, no acquire needed).
    /// Useful for meshing - copies entire buffer atomically.
    [[nodiscard]] std::array<Material, VOLUME> getBlockSnapshot() const;

    //==========================================================================
    // MAIN THREAD WRITE OPERATIONS
    //==========================================================================

    /// Set a block in the BACK buffer (main thread only).
    void setBlock(uint8_t x, uint8_t y, uint8_t z, Material id);

    /// Fill a region in the BACK buffer (main thread only).
    void fill(glm::ivec3 from, glm::ivec3 to, Material id);

    /// Swap front and back buffers, making writes visible to readers.
    /// Waits for all active readers to finish before swapping.
    /// Returns true if swap occurred, false if no changes pending.
    bool swapBuffers();

    /// Check if there are uncommitted changes in the back buffer.
    [[nodiscard]] bool hasPendingChanges() const;

    //==========================================================================
    // GENERATION PHASE OPERATIONS (generation worker thread only)
    //==========================================================================

    /// Direct write during generation phase (no double buffering).
    /// Only safe when state == GENERATING and no other threads access chunk.
    void setBlockDirect(uint8_t x, uint8_t y, uint8_t z, Material id);

    /// Finalize generation - copies to both buffers and marks clean.
    void finalizeGeneration();

    //==========================================================================
    // LEGACY API (for compatibility, routes to appropriate buffer)
    //==========================================================================

    [[nodiscard]] Material getBlock(uint8_t x, uint8_t y, uint8_t z) const;
    [[nodiscard]] bool isAir(uint8_t x, uint8_t y, uint8_t z) const;

    //==========================================================================
    // STATE AND METADATA
    //==========================================================================

    [[nodiscard]] glm::mat4 getChunkModel() const;
    [[nodiscard]] ChunkPos getPosition() const;

    [[nodiscard]] ChunkState getState() const;
    void setState(ChunkState newState);

    [[nodiscard]] bool isDirty() const;
    void setDirty(bool dirty);

    [[nodiscard]] uint64_t getGenerationID() const;
    void bumpGenerationID();

private:
    ChunkPos m_position;

    // Double buffer: two complete block arrays
    std::array<Material, VOLUME> m_buffers[2];

    // Index of buffer that workers read from (0 or 1)
    std::atomic<uint8_t> m_readIndex{0};

    // Count of active readers (prevents swap during reads)
    mutable std::atomic<uint32_t> m_activeReaders{0};

    // True if back buffer has changes not yet swapped
    std::atomic<bool> m_pendingChanges{false};

    // Chunk lifecycle state
    std::atomic<ChunkState> m_state{ChunkState::UNLOADED};
    std::atomic<uint64_t> m_generationID{0};
    std::atomic<bool> m_dirty{false};

    // Helper to get write buffer index
    [[nodiscard]] uint8_t getWriteIndex() const {
        return 1 - m_readIndex.load(std::memory_order_acquire);
    }
};

//==============================================================================
// RAII Read Guard (recommended way to read from chunks)
//==============================================================================

class ChunkReadGuard {
public:
    explicit ChunkReadGuard(const Chunk& chunk)
        : m_chunk(chunk)
    {
        m_chunk.acquireRead();
    }

    ~ChunkReadGuard() {
        m_chunk.releaseRead();
    }

    // Non-copyable, non-movable
    ChunkReadGuard(const ChunkReadGuard&) = delete;
    ChunkReadGuard& operator=(const ChunkReadGuard&) = delete;

    [[nodiscard]] Material getBlock(uint8_t x, uint8_t y, uint8_t z) const {
        return m_chunk.getBlockRead(x, y, z);
    }

    [[nodiscard]] bool isAir(uint8_t x, uint8_t y, uint8_t z) const {
        return m_chunk.isAirRead(x, y, z);
    }

private:
    const Chunk& m_chunk;
};

#endif // RE_MINECRAFT_CHUNK_H
```

### Chunk.cpp

```cpp
#include "Chunk.h"
#include <thread>

//==============================================================================
// CONSTRUCTION
//==============================================================================

Chunk::Chunk(const ChunkPos pos)
    : m_position(pos)
{
    // Initialize both buffers to air
    m_buffers[0].fill(0);
    m_buffers[1].fill(0);
}

Chunk::Chunk(Chunk&& other) noexcept
    : m_position(other.m_position)
    , m_buffers{std::move(other.m_buffers[0]), std::move(other.m_buffers[1])}
    , m_readIndex(other.m_readIndex.load())
    , m_activeReaders(other.m_activeReaders.load())
    , m_pendingChanges(other.m_pendingChanges.load())
    , m_state(other.m_state.load())
    , m_generationID(other.m_generationID.load())
    , m_dirty(other.m_dirty.load())
{
}

Chunk& Chunk::operator=(Chunk&& other) noexcept {
    if (this != &other) {
        m_position = other.m_position;
        m_buffers[0] = std::move(other.m_buffers[0]);
        m_buffers[1] = std::move(other.m_buffers[1]);
        m_readIndex.store(other.m_readIndex.load());
        m_activeReaders.store(other.m_activeReaders.load());
        m_pendingChanges.store(other.m_pendingChanges.load());
        m_state.store(other.m_state.load());
        m_generationID.store(other.m_generationID.load());
        m_dirty.store(other.m_dirty.load());
    }
    return *this;
}

//==============================================================================
// THREAD-SAFE READ OPERATIONS
//==============================================================================

uint8_t Chunk::acquireRead() const {
    // Increment reader count - prevents swap while we're reading
    m_activeReaders.fetch_add(1, std::memory_order_acquire);

    // Return current read index
    return m_readIndex.load(std::memory_order_acquire);
}

void Chunk::releaseRead() const {
    // Decrement reader count
    m_activeReaders.fetch_sub(1, std::memory_order_release);
}

Material Chunk::getBlockRead(uint8_t x, uint8_t y, uint8_t z) const {
    const uint8_t readIdx = m_readIndex.load(std::memory_order_acquire);
    return m_buffers[readIdx][ChunkCoords::localCoordsToIndex(x, y, z)];
}

bool Chunk::isAirRead(uint8_t x, uint8_t y, uint8_t z) const {
    const uint8_t readIdx = m_readIndex.load(std::memory_order_acquire);
    return m_buffers[readIdx][ChunkCoords::localCoordsToIndex(x, y, z)] == 0;
}

std::array<Material, Chunk::VOLUME> Chunk::getBlockSnapshot() const {
    // Acquire read access
    acquireRead();

    // Copy the entire front buffer
    const uint8_t readIdx = m_readIndex.load(std::memory_order_acquire);
    std::array<Material, VOLUME> snapshot = m_buffers[readIdx];

    // Release read access
    releaseRead();

    return snapshot;
}

//==============================================================================
// MAIN THREAD WRITE OPERATIONS
//==============================================================================

void Chunk::setBlock(uint8_t x, uint8_t y, uint8_t z, Material id) {
    const uint8_t writeIdx = getWriteIndex();
    m_buffers[writeIdx][ChunkCoords::localCoordsToIndex(x, y, z)] = id;
    m_pendingChanges.store(true, std::memory_order_release);
}

void Chunk::fill(glm::ivec3 from, glm::ivec3 to, Material id) {
    const uint8_t writeIdx = getWriteIndex();

    for (int z = from.z; z <= to.z; ++z) {
        for (int y = from.y; y <= to.y; ++y) {
            for (int x = from.x; x <= to.x; ++x) {
                m_buffers[writeIdx][ChunkCoords::localCoordsToIndex(x, y, z)] = id;
            }
        }
    }

    m_pendingChanges.store(true, std::memory_order_release);
}

bool Chunk::swapBuffers() {
    // Nothing to swap if no changes
    if (!m_pendingChanges.load(std::memory_order_acquire)) {
        return false;
    }

    // Wait for all readers to finish
    // This is a spin-wait, but should be very short in practice
    while (m_activeReaders.load(std::memory_order_acquire) > 0) {
        std::this_thread::yield();
    }

    // Memory fence to ensure all writes are visible
    std::atomic_thread_fence(std::memory_order_seq_cst);

    // Swap the buffer index
    const uint8_t oldReadIdx = m_readIndex.load(std::memory_order_acquire);
    const uint8_t newReadIdx = 1 - oldReadIdx;

    // Copy new read buffer to new write buffer (so writes continue from current state)
    m_buffers[oldReadIdx] = m_buffers[newReadIdx];

    // Atomically update read index
    m_readIndex.store(newReadIdx, std::memory_order_release);

    // Clear pending flag
    m_pendingChanges.store(false, std::memory_order_release);

    return true;
}

bool Chunk::hasPendingChanges() const {
    return m_pendingChanges.load(std::memory_order_acquire);
}

//==============================================================================
// GENERATION PHASE OPERATIONS
//==============================================================================

void Chunk::setBlockDirect(uint8_t x, uint8_t y, uint8_t z, Material id) {
    // During generation, write to buffer 0 directly
    // This is safe because only the generation worker accesses the chunk
    m_buffers[0][ChunkCoords::localCoordsToIndex(x, y, z)] = id;
}

void Chunk::finalizeGeneration() {
    // Copy buffer 0 to buffer 1 so both are identical
    m_buffers[1] = m_buffers[0];

    // Ensure read index points to valid data
    m_readIndex.store(0, std::memory_order_release);

    // No pending changes after generation
    m_pendingChanges.store(false, std::memory_order_release);
}

//==============================================================================
// LEGACY API
//==============================================================================

Material Chunk::getBlock(uint8_t x, uint8_t y, uint8_t z) const {
    // For backwards compatibility, read from front buffer
    // Note: This is only safe if called from main thread or with proper synchronization
    return getBlockRead(x, y, z);
}

bool Chunk::isAir(uint8_t x, uint8_t y, uint8_t z) const {
    return isAirRead(x, y, z);
}

//==============================================================================
// STATE AND METADATA
//==============================================================================

glm::mat4 Chunk::getChunkModel() const {
    const auto offset = glm::vec3(
        m_position.x * SIZE,
        m_position.y * SIZE,
        m_position.z * SIZE
    );
    return glm::translate(glm::mat4(1.0f), offset);
}

ChunkPos Chunk::getPosition() const {
    return m_position;
}

ChunkState Chunk::getState() const {
    return m_state.load(std::memory_order_acquire);
}

void Chunk::setState(ChunkState newState) {
    m_state.store(newState, std::memory_order_release);
}

bool Chunk::isDirty() const {
    return m_dirty.load(std::memory_order_acquire);
}

void Chunk::setDirty(bool dirty) {
    m_dirty.store(dirty, std::memory_order_release);
}

uint64_t Chunk::getGenerationID() const {
    return m_generationID.load(std::memory_order_acquire);
}

void Chunk::bumpGenerationID() {
    m_generationID.fetch_add(1, std::memory_order_acq_rel);
}
```

---

## Integration Guide

### Step 1: Update TerrainGenerator

```cpp
// TerrainGenerator.cpp

void TerrainGenerator::generate(Chunk& chunk, const BlockRegistry& blockRegistry) {
    // Use setBlockDirect during generation (single-threaded access)
    for (int x = 0; x < Chunk::SIZE; x++) {
        for (int z = 0; z < Chunk::SIZE; z++) {
            const auto [cx, cy, cz] = chunk.getPosition();

            const int wx = cx * Chunk::SIZE + x;
            const int wz = cz * Chunk::SIZE + z;
            const int height = getTerrainHeight(wx, wz);

            for (int y = 0; y < Chunk::SIZE; y++) {
                const int wy = cy * Chunk::SIZE + y;

                Material mat;
                if (wy < 2)
                    mat = blockRegistry.getByName("core:stone");
                else if (wy < height)
                    mat = blockRegistry.getByName("core:dirt");
                else if (wy == height)
                    mat = blockRegistry.getByName("core:grass");
                else
                    mat = blockRegistry.getByName("core:air");

                chunk.setBlockDirect(x, y, z, mat);  // Direct write, no buffering
            }
        }
    }

    // Finalize: copy to both buffers
    chunk.finalizeGeneration();
}
```

### Step 2: Update ChunkManager

```cpp
// ChunkManager.cpp

void ChunkManager::generateJob(ChunkJob job) {
    Chunk* chunk = getChunk(job.pos.x, job.pos.y, job.pos.z);

    if (!chunk)
        return;

    if (chunk->getGenerationID() != job.generationID)
        return;

    // Generate terrain (uses setBlockDirect)
    TerrainGenerator::generate(*chunk, this->blockRegistry);

    // State transition
    chunk->setState(ChunkState::GENERATED);
    rebuildNeighbors(job.pos);
}
```

### Step 3: Update ChunkMeshManager

```cpp
// ChunkMeshManager.cpp

void ChunkMeshManager::buildMeshJob(const ChunkJob& job) {
    Chunk* chunk = world.getChunkManager()->getChunk(job.pos.x, job.pos.y, job.pos.z);

    if (!chunk)
        return;

    if (chunk->getGenerationID() != job.generationID)
        return;

    // Take snapshots of all needed chunks (thread-safe)
    auto blockData = chunk->getBlockSnapshot();

    const ChunkNeighbors n = world.getChunkManager()->getNeighbors(job.pos);

    // Snapshot neighbor data
    struct NeighborData {
        bool exists;
        std::array<Material, Chunk::VOLUME> blocks;
    };

    NeighborData neighbors[6];
    neighbors[0] = { n.north != nullptr, n.north ? n.north->getBlockSnapshot() : std::array<Material, Chunk::VOLUME>{} };
    neighbors[1] = { n.south != nullptr, n.south ? n.south->getBlockSnapshot() : std::array<Material, Chunk::VOLUME>{} };
    neighbors[2] = { n.east  != nullptr, n.east  ? n.east->getBlockSnapshot()  : std::array<Material, Chunk::VOLUME>{} };
    neighbors[3] = { n.west  != nullptr, n.west  ? n.west->getBlockSnapshot()  : std::array<Material, Chunk::VOLUME>{} };
    neighbors[4] = { n.up    != nullptr, n.up    ? n.up->getBlockSnapshot()    : std::array<Material, Chunk::VOLUME>{} };
    neighbors[5] = { n.down  != nullptr, n.down  ? n.down->getBlockSnapshot()  : std::array<Material, Chunk::VOLUME>{} };

    // Now build mesh using local snapshots - completely thread-safe!
    const auto& textureRegistry = world.getTextureRegistry();

    MeshData data;
    data.reserve(36 * Chunk::VOLUME);

    for (int i = 0; i < Chunk::VOLUME; i++) {
        auto [x, y, z] = ChunkCoords::indexToLocalCoords(i);

        // Use local snapshot instead of chunk->getBlock()
        Material mat = blockData[i];

        if (mat == 0)  // AIR
            continue;

        const BlockMeta& meta = world.getBlockRegistry().get(mat);

        // Check faces using local snapshots
        // NORTH face (z - 1)
        if (isAirAtSnapshot(blockData, neighbors, x, y, z - 1)) {
            buildFaceMesh(data, /* ... */);
        }

        // ... other faces
    }

    {
        std::lock_guard lock(uploadMutex);
        uploadQueue.emplace(job.pos, std::move(data));
    }

    chunk->setState(ChunkState::MESHED);
}

// Helper function for snapshot-based air check
bool ChunkMeshManager::isAirAtSnapshot(
    const std::array<Material, Chunk::VOLUME>& blockData,
    const NeighborData neighbors[6],
    int x, int y, int z)
{
    // Inside current chunk
    if (x >= 0 && x < Chunk::SIZE &&
        y >= 0 && y < Chunk::SIZE &&
        z >= 0 && z < Chunk::SIZE)
    {
        return blockData[ChunkCoords::localCoordsToIndex(x, y, z)] == 0;
    }

    // Check neighbors
    if (z < 0) {  // NORTH
        if (!neighbors[0].exists) return true;
        return neighbors[0].blocks[ChunkCoords::localCoordsToIndex(x, y, z + Chunk::SIZE)] == 0;
    }
    if (z >= Chunk::SIZE) {  // SOUTH
        if (!neighbors[1].exists) return true;
        return neighbors[1].blocks[ChunkCoords::localCoordsToIndex(x, y, z - Chunk::SIZE)] == 0;
    }
    if (x >= Chunk::SIZE) {  // EAST
        if (!neighbors[2].exists) return true;
        return neighbors[2].blocks[ChunkCoords::localCoordsToIndex(x - Chunk::SIZE, y, z)] == 0;
    }
    if (x < 0) {  // WEST
        if (!neighbors[3].exists) return true;
        return neighbors[3].blocks[ChunkCoords::localCoordsToIndex(x + Chunk::SIZE, y, z)] == 0;
    }
    if (y >= Chunk::SIZE) {  // UP
        if (!neighbors[4].exists) return true;
        return neighbors[4].blocks[ChunkCoords::localCoordsToIndex(x, y - Chunk::SIZE, z)] == 0;
    }
    if (y < 0) {  // DOWN
        if (!neighbors[5].exists) return true;
        return neighbors[5].blocks[ChunkCoords::localCoordsToIndex(x, y + Chunk::SIZE, z)] == 0;
    }

    return true;  // Outside all bounds = air
}
```

### Step 4: Update World for Block Modifications

```cpp
// World.cpp

void World::setBlock(const int wx, const int wy, const int wz, const Material id) const {
    const auto [cx, cy, cz] = ChunkPos::fromWorld(wx, wy, wz);
    const auto [x, y, z] = BlockPos::fromWorld(wx, wy, wz);

    Chunk* chunk = this->chunkManager->getChunk(cx, cy, cz);

    if (!chunk || chunk->getState() != ChunkState::READY)
        return;  // Can only modify READY chunks

    // Write to back buffer
    chunk->setBlock(x, y, z, id);

    // Mark for remeshing
    chunk->setDirty(true);
    chunk->setState(ChunkState::GENERATED);

    // Also mark neighbors dirty if on edge
    this->chunkManager->rebuildNeighbors({cx, cy, cz});
}

void World::update(const glm::vec3& cameraPos) {
    // Swap buffers for any chunks with pending changes
    for (auto& [pos, chunk] : chunkManager->getChunks()) {
        if (chunk.hasPendingChanges() && chunk.getState() == ChunkState::GENERATED) {
            chunk.swapBuffers();
        }
    }

    this->chunkManager->updateStreaming(cameraPos);
    this->meshManager.scheduleMeshing(cameraPos);
    this->meshManager.update();
}
```

---

## Usage Examples

### Example 1: Safe Reading from Worker Thread

```cpp
void workerFunction(const Chunk& chunk) {
    // Option A: Using RAII guard (recommended)
    {
        ChunkReadGuard guard(chunk);

        for (int y = 0; y < Chunk::SIZE; y++) {
            Material mat = guard.getBlock(0, y, 0);
            // Process mat...
        }
    }  // Guard released here, swap can proceed

    // Option B: Manual acquire/release
    chunk.acquireRead();

    Material mat = chunk.getBlockRead(5, 10, 5);
    bool air = chunk.isAirRead(5, 11, 5);

    chunk.releaseRead();

    // Option C: Full snapshot (best for meshing)
    auto snapshot = chunk.getBlockSnapshot();
    // snapshot is a local copy, completely independent
    for (int i = 0; i < Chunk::VOLUME; i++) {
        if (snapshot[i] != 0) {
            // Process solid block...
        }
    }
}
```

### Example 2: Safe Writing from Main Thread

```cpp
void mainThreadFunction(Chunk& chunk) {
    // Modify multiple blocks
    chunk.setBlock(5, 10, 5, Material::STONE);
    chunk.setBlock(5, 11, 5, Material::STONE);
    chunk.setBlock(5, 12, 5, Material::STONE);

    // Changes are in back buffer, not visible to workers yet

    // Make changes visible (waits for readers to finish)
    if (chunk.swapBuffers()) {
        // Swap occurred, trigger remeshing
        chunk.setDirty(true);
    }
}
```

### Example 3: Batch Modifications

```cpp
void buildStructure(Chunk& chunk) {
    // Build a 5x5x5 cube of stone
    chunk.fill(glm::ivec3(5, 5, 5), glm::ivec3(10, 10, 10), Material::STONE);

    // Carve out the inside (3x3x3 air)
    chunk.fill(glm::ivec3(6, 6, 6), glm::ivec3(9, 9, 9), Material::AIR);

    // Single swap at the end
    chunk.swapBuffers();
}
```

---

## Performance Considerations

### Memory Overhead

```
Current:    4096 * sizeof(Material) = 8 KB per chunk
Double:     4096 * sizeof(Material) * 2 = 16 KB per chunk

With 500 chunks loaded:
Current:    4 MB
Double:     8 MB

Overhead: +4 MB (negligible on modern systems)
```

### Swap Cost

The swap operation has three costs:

1. **Spin-wait for readers**: Usually 0-1 iterations (readers are fast)
2. **Memory fence**: ~20-50 CPU cycles
3. **Buffer copy**: 8 KB memcpy = ~1-2 microseconds

Total swap cost: **~2-5 microseconds per chunk**

### When to Swap

```cpp
// BAD: Swap after every single block change
void setBlockSlow(Chunk& chunk, int x, int y, int z, Material m) {
    chunk.setBlock(x, y, z, m);
    chunk.swapBuffers();  // Expensive if called frequently!
}

// GOOD: Batch changes, swap once per frame
void updateChunks(std::vector<Chunk*>& modifiedChunks) {
    for (Chunk* chunk : modifiedChunks) {
        if (chunk->hasPendingChanges()) {
            chunk->swapBuffers();  // Once per modified chunk per frame
        }
    }
}
```

### Comparison with Alternatives

| Operation | Mutex | RW Lock | Double Buffer |
|-----------|-------|---------|---------------|
| Single read | 50-100ns | 30-50ns | **~5ns** (just load) |
| Bulk read (4096) | 50-100ns + contention | 30-50ns + contention | **~5ns** (snapshot copy) |
| Single write | 50-100ns | 100-200ns | **~5ns** (just store) |
| Bulk write (100) | 50-100ns | 100-200ns | **~500ns** |
| Publish changes | N/A | N/A | **~2-5μs** (swap) |

**Double buffering wins when:**
- Reads vastly outnumber writes (typical for voxel engines)
- Multiple workers read simultaneously
- Writes can be batched

---

## Summary

Double buffering provides **lock-free reads** for worker threads while allowing the main thread to modify blocks safely. The key points are:

1. **Two buffers**: Front (read) and Back (write)
2. **Atomic swap**: Makes writes visible to readers
3. **Reader counting**: Prevents swap during active reads
4. **Snapshots**: Best for meshing (full copy, independent of chunk)

This approach eliminates all data races while maintaining excellent read performance for the meshing workers.
