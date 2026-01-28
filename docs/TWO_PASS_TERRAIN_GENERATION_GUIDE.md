# Two-Pass Terrain Generation Guide

This document provides a complete implementation guide for solving the **cross-chunk structure generation problem** using a two-pass generation system with a decoration phase.

---

## Table of Contents

1. [Problem Statement](#problem-statement)
2. [Solution Overview](#solution-overview)
3. [Architecture Changes](#architecture-changes)
4. [Implementation Steps](#implementation-steps)
5. [Pseudo-Code Reference](#pseudo-code-reference)
6. [Threading Considerations](#threading-considerations)
7. [Edge Cases and Gotchas](#edge-cases-and-gotchas)
8. [Testing Strategy](#testing-strategy)

---

## Problem Statement

### The Cross-Chunk Structure Problem

When generating terrain procedurally in chunks (16x16x16 blocks), structures like trees can extend beyond the boundaries of the chunk where they originate.

```
        Chunk A              Chunk B (not yet generated)
    ┌───────────────┐    ┌───────────────┐
    │               │    │               │
    │       🌳      │    │               │
    │      ┌───┐    │    │               │
    │      │ T │────│────│──> Tree leaves cut off!
    │      │ R │    │    │               │
    │      │ E │    │    │               │
    │      │ E │    │    │               │
    └──────┴───┴────┘    └───────────────┘
         x=14                 x=0
```

**Current behavior:**
- Tree origin is at x=14 in Chunk A
- Tree canopy extends to x=17, 18, 19 (into Chunk B)
- `setBlockDirect()` only works within chunk bounds (0-15)
- Tree is cut in half

### Why This Happens

In the current architecture:

```cpp
void TerrainGenerator::generate(Chunk& chunk) {
    // 1. Generate terrain (works fine)
    generateTerrain(chunk);

    // 2. Place structures (PROBLEM!)
    placeStructures(chunk);  // Can only modify THIS chunk
}
```

The `Chunk` class only allows setting blocks within its own boundaries:

```cpp
void Chunk::setBlockDirect(const BlockPos& pos, Material mat) {
    // pos.x, pos.y, pos.z must be in range [0, 15]
    if (pos.x < 0 || pos.x >= 16) return;  // Overflow blocks are lost!
    // ...
}
```

---

## Solution Overview

### Two-Pass Generation Architecture

Split chunk generation into two distinct phases:

| Phase | Name | Purpose | Neighbor Requirement |
|-------|------|---------|---------------------|
| Pass 1 | **Terrain** | Generate base terrain (stone, dirt, grass) | None |
| Pass 2 | **Decoration** | Place structures (trees, ores, features) | All 26 neighbors must be at Pass 1+ |

```
                        GENERATION FLOW
                        ═══════════════

    ┌─────────────┐     ┌─────────────┐     ┌─────────────┐
    │   UNLOADED  │────▶│  TERRAIN    │────▶│  DECORATED  │
    │             │     │  (Pass 1)   │     │  (Pass 2)   │
    └─────────────┘     └─────────────┘     └─────────────┘
                              │                    │
                              │                    ▼
                              │             ┌─────────────┐
                              │             │   MESHING   │
                              │             └─────────────┘
                              │                    │
                              │                    ▼
                              │             ┌─────────────┐
                              └────────────▶│    READY    │
                                            └─────────────┘
```

### Key Insight: Neighbor Access During Decoration

During Pass 2, the chunk being decorated has access to all its neighbors (which are guaranteed to have terrain). This means structures can:

1. **Read** neighbor block data (to check terrain height, find valid placement spots)
2. **Write** to neighbor chunks (to place overflow blocks)

```
    Pass 2: Decorating Chunk A
    ══════════════════════════

    Chunk A can now write to neighbors B, C, D...

         ┌───┬───┬───┐
         │ D │ E │ F │    All neighbors have
         ├───┼───┼───┤    completed Pass 1
         │ G │ A │ B │    (terrain exists)
         ├───┼───┼───┤
         │ H │ I │ C │
         └───┴───┴───┘
```

---

## Architecture Changes

### 1. New Chunk States

Update `ChunkState.h` to include the new generation phases:

```cpp
// Current states
enum class ChunkState {
    UNLOADED,
    GENERATING,    // Remove or repurpose
    GENERATED,     // Remove or repurpose
    MESHING,
    MESHED,
    READY
};

// New states (two-pass system)
enum class ChunkState {
    UNLOADED,           // Not in memory

    // Pass 1: Terrain
    TERRAIN_PENDING,    // Queued for terrain generation
    TERRAIN_GENERATING, // Worker thread generating terrain
    TERRAIN_DONE,       // Terrain complete, waiting for decoration

    // Pass 2: Decoration
    DECOR_PENDING,      // Queued for decoration
    DECOR_GENERATING,   // Worker thread placing structures
    DECOR_DONE,         // Fully generated

    // Meshing (unchanged)
    MESHING,
    MESHED,
    READY
};
```

### 2. ChunkManager Responsibilities

The `ChunkManager` gains new responsibilities:

| Responsibility | Description |
|----------------|-------------|
| Track generation phase | Know which pass each chunk is in |
| Check decoration eligibility | Verify all 26 neighbors have terrain |
| Provide neighbor access | Give `TerrainGenerator` access to neighbor chunks |
| Coordinate write locking | Ensure thread-safe writes to multiple chunks |

### 3. TerrainGenerator Changes

Split into two methods:

```cpp
class TerrainGenerator {
public:
    // Pass 1: Only terrain, no structures
    void generateTerrain(Chunk& chunk);

    // Pass 2: Structures that may span chunks
    void decorateChunk(Chunk& chunk, NeighborAccess& neighbors);
};
```

### 4. New NeighborAccess Class

A helper class that provides controlled access to neighboring chunks:

```cpp
class NeighborAccess {
public:
    // Get block from any neighbor (or self)
    Material getBlock(int worldX, int worldY, int worldZ) const;

    // Set block in any neighbor (or self) - THE KEY FEATURE
    void setBlock(int worldX, int worldY, int worldZ, Material mat);

private:
    Chunk* centerChunk;
    std::array<Chunk*, 27> chunks;  // 3x3x3 neighborhood
};
```

---

## Implementation Steps

### Step 1: Update ChunkState Enum

**File:** `src/Content/Chunk/ChunkState.h`

```cpp
#ifndef RE_MINECRAFT_CHUNKSTATE_H
#define RE_MINECRAFT_CHUNKSTATE_H

enum class ChunkState {
    UNLOADED,

    // Pass 1: Base terrain
    TERRAIN_PENDING,
    TERRAIN_GENERATING,
    TERRAIN_DONE,

    // Pass 2: Decoration (structures)
    DECOR_PENDING,
    DECOR_GENERATING,
    DECOR_DONE,

    // Meshing
    MESHING,
    MESHED,
    READY
};

// Helper to check if terrain is complete
inline bool hasTerrainComplete(ChunkState state) {
    return state >= ChunkState::TERRAIN_DONE;
}

// Helper to check if fully generated
inline bool isFullyGenerated(ChunkState state) {
    return state >= ChunkState::DECOR_DONE;
}

#endif
```

### Step 2: Create NeighborAccess Class

**File:** `src/Content/TerrainGenerator/NeighborAccess.h`

```cpp
#ifndef RE_MINECRAFT_NEIGHBORACCESS_H
#define RE_MINECRAFT_NEIGHBORACCESS_H

#include <array>
#include "Content/Chunk/Chunk.h"
#include "Content/Chunk/ChunkPos.h"

class NeighborAccess {
public:
    // Constructor takes the center chunk position and a lookup function
    NeighborAccess(const ChunkPos& centerPos,
                   std::function<Chunk*(const ChunkPos&)> getChunk);

    // Check if all neighbors are available (have terrain)
    bool allNeighborsReady() const;

    // Get the center chunk
    Chunk* getCenter() const { return m_chunks[13]; }  // Index 13 = center of 3x3x3

    // World coordinate block access (handles cross-chunk automatically)
    Material getBlock(int worldX, int worldY, int worldZ) const;
    void setBlock(int worldX, int worldY, int worldZ, Material mat);

    // Mark all affected chunks as dirty (for remeshing)
    void markDirtyChunks();

private:
    ChunkPos m_centerPos;
    std::array<Chunk*, 27> m_chunks;        // 3x3x3 grid
    std::array<bool, 27> m_chunkModified;   // Track which chunks were written to

    // Convert 3D offset (-1,0,1) to array index (0-26)
    static int offsetToIndex(int dx, int dy, int dz) {
        return (dx + 1) + (dy + 1) * 3 + (dz + 1) * 9;
    }

    // Get chunk containing world position
    Chunk* getChunkForWorldPos(int wx, int wy, int wz) const;
};

#endif
```

**File:** `src/Content/TerrainGenerator/NeighborAccess.cpp`

```cpp
#include "NeighborAccess.h"

NeighborAccess::NeighborAccess(const ChunkPos& centerPos,
                               std::function<Chunk*(const ChunkPos&)> getChunk)
    : m_centerPos(centerPos)
{
    m_chunkModified.fill(false);

    // Populate the 3x3x3 neighborhood
    for (int dy = -1; dy <= 1; dy++) {
        for (int dz = -1; dz <= 1; dz++) {
            for (int dx = -1; dx <= 1; dx++) {
                ChunkPos neighborPos = {
                    centerPos.x + dx,
                    centerPos.y + dy,
                    centerPos.z + dz
                };
                int index = offsetToIndex(dx, dy, dz);
                m_chunks[index] = getChunk(neighborPos);
            }
        }
    }
}

bool NeighborAccess::allNeighborsReady() const {
    for (Chunk* chunk : m_chunks) {
        if (chunk == nullptr) return false;
        if (!hasTerrainComplete(chunk->getState())) return false;
    }
    return true;
}

Material NeighborAccess::getBlock(int wx, int wy, int wz) const {
    Chunk* chunk = getChunkForWorldPos(wx, wy, wz);
    if (!chunk) return Material(0);  // Air for out-of-range

    BlockPos local = {wx & 15, wy & 15, wz & 15};
    return chunk->getBlock(local);
}

void NeighborAccess::setBlock(int wx, int wy, int wz, Material mat) {
    // Calculate which chunk this world position belongs to
    int cx = wx >> 4;  // Divide by 16
    int cy = wy >> 4;
    int cz = wz >> 4;

    // Calculate offset from center chunk
    int dx = cx - m_centerPos.x;
    int dy = cy - m_centerPos.y;
    int dz = cz - m_centerPos.z;

    // Check if within our 3x3x3 neighborhood
    if (dx < -1 || dx > 1 || dy < -1 || dy > 1 || dz < -1 || dz > 1) {
        // Structure extends beyond immediate neighbors - skip this block
        // (Very rare for normal structures, could log warning)
        return;
    }

    int index = offsetToIndex(dx, dy, dz);
    Chunk* chunk = m_chunks[index];

    if (chunk && hasTerrainComplete(chunk->getState())) {
        BlockPos local = {wx & 15, wy & 15, wz & 15};
        chunk->setBlockDirect(local, mat);
        m_chunkModified[index] = true;
    }
}

Chunk* NeighborAccess::getChunkForWorldPos(int wx, int wy, int wz) const {
    int cx = wx >> 4;
    int cy = wy >> 4;
    int cz = wz >> 4;

    int dx = cx - m_centerPos.x;
    int dy = cy - m_centerPos.y;
    int dz = cz - m_centerPos.z;

    if (dx < -1 || dx > 1 || dy < -1 || dy > 1 || dz < -1 || dz > 1) {
        return nullptr;
    }

    return m_chunks[offsetToIndex(dx, dy, dz)];
}

void NeighborAccess::markDirtyChunks() {
    for (int i = 0; i < 27; i++) {
        if (m_chunkModified[i] && m_chunks[i]) {
            m_chunks[i]->markDirty();
        }
    }
}
```

### Step 3: Split TerrainGenerator

**File:** `src/Content/TerrainGenerator/TerrainGenerator.h`

```cpp
#ifndef RE_MINECRAFT_TERRAINGENERATOR_H
#define RE_MINECRAFT_TERRAINGENERATOR_H

#include "Content/Chunk/Chunk.h"
#include "NeighborAccess.h"
#include "Content/PrefabRegistry/PrefabRegistry.h"
#include <FastNoiseLite.h>

class TerrainGenerator {
public:
    TerrainGenerator(int seed, PrefabRegistry& prefabRegistry);

    // Pass 1: Generate base terrain only (stone, dirt, grass)
    // No structures, no features that span chunks
    void generateTerrain(Chunk& chunk);

    // Pass 2: Place decorations (trees, ores, flowers, etc.)
    // Has access to neighboring chunks for cross-boundary placement
    void decorate(Chunk& chunk, NeighborAccess& neighbors);

private:
    FastNoiseLite m_noise;
    PrefabRegistry& m_prefabRegistry;
    int m_seed;

    // Terrain helpers
    int getTerrainHeight(int worldX, int worldZ) const;

    // Decoration helpers
    void placeTree(NeighborAccess& neighbors, int worldX, int worldY, int worldZ);
    void placePrefab(NeighborAccess& neighbors, const PrefabMeta& prefab,
                     int worldX, int worldY, int worldZ);

    // Deterministic random for decoration placement
    uint32_t getDecorationSeed(int chunkX, int chunkZ) const;
};

#endif
```

**File:** `src/Content/TerrainGenerator/TerrainGenerator.cpp`

```cpp
#include "TerrainGenerator.h"
#include <random>

TerrainGenerator::TerrainGenerator(int seed, PrefabRegistry& prefabRegistry)
    : m_seed(seed)
    , m_prefabRegistry(prefabRegistry)
{
    m_noise.SetSeed(seed);
    m_noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    m_noise.SetFrequency(0.015f);
}

//==============================================================================
// PASS 1: TERRAIN GENERATION
//==============================================================================

void TerrainGenerator::generateTerrain(Chunk& chunk) {
    const ChunkPos& pos = chunk.getPosition();

    for (int lx = 0; lx < 16; lx++) {
        for (int lz = 0; lz < 16; lz++) {
            int worldX = pos.x * 16 + lx;
            int worldZ = pos.z * 16 + lz;
            int terrainHeight = getTerrainHeight(worldX, worldZ);

            for (int ly = 0; ly < 16; ly++) {
                int worldY = pos.y * 16 + ly;

                Material mat;
                if (worldY < terrainHeight - 3) {
                    mat = Material::fromBlockId(BlockId::STONE);
                } else if (worldY < terrainHeight) {
                    mat = Material::fromBlockId(BlockId::DIRT);
                } else if (worldY == terrainHeight) {
                    mat = Material::fromBlockId(BlockId::GRASS);
                } else {
                    mat = Material::fromBlockId(BlockId::AIR);
                }

                chunk.setBlockDirect({lx, ly, lz}, mat);
            }
        }
    }

    // NOTE: No structure placement here!
    // Trees and other decorations are placed in Pass 2
}

int TerrainGenerator::getTerrainHeight(int worldX, int worldZ) const {
    float noiseValue = m_noise.GetNoise(
        static_cast<float>(worldX),
        static_cast<float>(worldZ)
    );
    return 64 + static_cast<int>(noiseValue * 8.0f);
}

//==============================================================================
// PASS 2: DECORATION
//==============================================================================

void TerrainGenerator::decorate(Chunk& chunk, NeighborAccess& neighbors) {
    const ChunkPos& pos = chunk.getPosition();

    // Use deterministic seed based on chunk position
    // This ensures the same trees are placed regardless of generation order
    uint32_t seed = getDecorationSeed(pos.x, pos.z);
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> xDist(2, 13);  // Avoid edges
    std::uniform_int_distribution<int> zDist(2, 13);
    std::uniform_int_distribution<int> treeCount(0, 2);

    int numTrees = treeCount(rng);

    for (int i = 0; i < numTrees; i++) {
        int lx = xDist(rng);
        int lz = zDist(rng);
        int worldX = pos.x * 16 + lx;
        int worldZ = pos.z * 16 + lz;

        // Find ground level using neighbor access (can check adjacent chunks)
        int groundY = findGroundLevel(neighbors, worldX, worldZ);

        if (groundY > 0) {
            Material groundBlock = neighbors.getBlock(worldX, groundY, worldZ);

            // Only place trees on grass
            if (groundBlock.getBlockId() == BlockId::GRASS) {
                placeTree(neighbors, worldX, groundY + 1, worldZ);
            }
        }
    }

    // Mark any chunks that were modified
    neighbors.markDirtyChunks();
}

uint32_t TerrainGenerator::getDecorationSeed(int chunkX, int chunkZ) const {
    // Combine world seed with chunk position for deterministic placement
    return static_cast<uint32_t>(m_seed) ^
           (static_cast<uint32_t>(chunkX) * 73856093u) ^
           (static_cast<uint32_t>(chunkZ) * 19349663u);
}

void TerrainGenerator::placeTree(NeighborAccess& neighbors,
                                  int worldX, int worldY, int worldZ) {
    // Get tree prefab
    const PrefabMeta* treePrefab = m_prefabRegistry.get("oak_tree_1");
    if (!treePrefab) return;

    placePrefab(neighbors, *treePrefab, worldX, worldY, worldZ);
}

void TerrainGenerator::placePrefab(NeighborAccess& neighbors,
                                    const PrefabMeta& prefab,
                                    int worldX, int worldY, int worldZ) {
    for (const auto& block : prefab.blocks) {
        int bx = worldX + block.offsetX;
        int by = worldY + block.offsetY;
        int bz = worldZ + block.offsetZ;

        // NeighborAccess handles cross-chunk placement automatically!
        neighbors.setBlock(bx, by, bz, block.material);
    }
}

int TerrainGenerator::findGroundLevel(NeighborAccess& neighbors,
                                       int worldX, int worldZ) const {
    // Search from top of world down to find solid ground
    // This works across chunk boundaries thanks to NeighborAccess
    for (int y = 128; y >= 0; y--) {
        Material mat = neighbors.getBlock(worldX, y, worldZ);
        if (mat.getBlockId() != BlockId::AIR) {
            return y;
        }
    }
    return -1;
}
```

### Step 4: Update ChunkManager

**File:** `src/Content/ChunkManager/ChunkManager.h` (additions)

```cpp
class ChunkManager {
public:
    // ... existing methods ...

    // New: Check if chunk can be decorated (all neighbors have terrain)
    bool canDecorate(const ChunkPos& pos) const;

    // New: Get chunk by position (for NeighborAccess)
    Chunk* getChunk(const ChunkPos& pos);

private:
    // ... existing members ...

    // Separate job queues for each pass
    ThreadPool<ChunkJob> m_terrainThreadPool;
    ThreadPool<ChunkJob> m_decorationThreadPool;  // Or reuse single pool

    // Job handlers
    void terrainJob(ChunkJob& job);
    void decorationJob(ChunkJob& job);

    // Check and queue chunks ready for decoration
    void updateDecorationQueue();
};
```

**File:** `src/Content/ChunkManager/ChunkManager.cpp` (key changes)

```cpp
void ChunkManager::requestChunk(const ChunkPos& pos) {
    auto chunk = std::make_unique<Chunk>(pos, m_blockRegistry);
    chunk->setState(ChunkState::TERRAIN_PENDING);

    ChunkJob job{pos, /* distance */, chunk->getGenerationId()};
    m_terrainThreadPool.enqueue(job);

    m_chunks[pos] = std::move(chunk);
}

void ChunkManager::terrainJob(ChunkJob& job) {
    Chunk* chunk = getChunk(job.pos);
    if (!chunk || chunk->getGenerationId() != job.generationId) {
        return;  // Chunk was unloaded
    }

    chunk->setState(ChunkState::TERRAIN_GENERATING);
    m_terrainGenerator.generateTerrain(*chunk);
    chunk->setState(ChunkState::TERRAIN_DONE);

    // Check if this chunk or neighbors can now be decorated
    notifyTerrainComplete(job.pos);
}

void ChunkManager::notifyTerrainComplete(const ChunkPos& pos) {
    // Check the completed chunk and all its neighbors
    // If any chunk now has all 26 neighbors with terrain, queue for decoration

    for (int dy = -1; dy <= 1; dy++) {
        for (int dz = -1; dz <= 1; dz++) {
            for (int dx = -1; dx <= 1; dx++) {
                ChunkPos checkPos = {pos.x + dx, pos.y + dy, pos.z + dz};
                tryQueueDecoration(checkPos);
            }
        }
    }
}

void ChunkManager::tryQueueDecoration(const ChunkPos& pos) {
    Chunk* chunk = getChunk(pos);
    if (!chunk) return;
    if (chunk->getState() != ChunkState::TERRAIN_DONE) return;

    if (canDecorate(pos)) {
        chunk->setState(ChunkState::DECOR_PENDING);

        ChunkJob job{pos, /* distance */, chunk->getGenerationId()};
        m_decorationThreadPool.enqueue(job);
    }
}

bool ChunkManager::canDecorate(const ChunkPos& pos) const {
    // Check all 26 neighbors (3x3x3 minus center)
    for (int dy = -1; dy <= 1; dy++) {
        for (int dz = -1; dz <= 1; dz++) {
            for (int dx = -1; dx <= 1; dx++) {
                ChunkPos neighborPos = {pos.x + dx, pos.y + dy, pos.z + dz};

                Chunk* neighbor = getChunk(neighborPos);
                if (!neighbor) return false;
                if (!hasTerrainComplete(neighbor->getState())) return false;
            }
        }
    }
    return true;
}

void ChunkManager::decorationJob(ChunkJob& job) {
    Chunk* chunk = getChunk(job.pos);
    if (!chunk || chunk->getGenerationId() != job.generationId) {
        return;
    }

    chunk->setState(ChunkState::DECOR_GENERATING);

    // Create NeighborAccess with chunk lookup function
    NeighborAccess neighbors(job.pos, [this](const ChunkPos& p) {
        return getChunk(p);
    });

    // Verify neighbors are still valid (could have been unloaded)
    if (!neighbors.allNeighborsReady()) {
        // Re-queue for later
        chunk->setState(ChunkState::TERRAIN_DONE);
        return;
    }

    m_terrainGenerator.decorate(*chunk, neighbors);
    chunk->setState(ChunkState::DECOR_DONE);

    // Trigger meshing for this chunk and any modified neighbors
    queueMeshing(job.pos);
}
```

### Step 5: Handle Concurrent Access

When multiple decoration jobs run in parallel, they might try to write to the same neighbor chunk. Add locking:

**Option A: Per-Chunk Mutex (Fine-grained)**

```cpp
class Chunk {
private:
    std::mutex m_writeMutex;  // For decoration writes

public:
    void setBlockThreadSafe(const BlockPos& pos, Material mat) {
        std::lock_guard<std::mutex> lock(m_writeMutex);
        setBlockDirect(pos, mat);
    }
};
```

**Option B: Decoration Scheduling (Coarse-grained)**

Only allow one decoration job per "region" at a time:

```cpp
class ChunkManager {
private:
    // Track which chunks are being decorated (including neighbors)
    std::unordered_set<ChunkPos> m_decorationLocks;
    std::mutex m_decorationLockMutex;

    bool tryAcquireDecorationLock(const ChunkPos& pos) {
        std::lock_guard<std::mutex> lock(m_decorationLockMutex);

        // Check if any chunk in 3x3x3 region is locked
        for (int dy = -1; dy <= 1; dy++) {
            for (int dz = -1; dz <= 1; dz++) {
                for (int dx = -1; dx <= 1; dx++) {
                    ChunkPos check = {pos.x + dx, pos.y + dy, pos.z + dz};
                    if (m_decorationLocks.count(check)) {
                        return false;  // Conflict
                    }
                }
            }
        }

        // Lock all chunks in region
        for (int dy = -1; dy <= 1; dy++) {
            for (int dz = -1; dz <= 1; dz++) {
                for (int dx = -1; dx <= 1; dx++) {
                    ChunkPos lock = {pos.x + dx, pos.y + dy, pos.z + dz};
                    m_decorationLocks.insert(lock);
                }
            }
        }
        return true;
    }

    void releaseDecorationLock(const ChunkPos& pos) {
        std::lock_guard<std::mutex> lock(m_decorationLockMutex);
        for (int dy = -1; dy <= 1; dy++) {
            for (int dz = -1; dz <= 1; dz++) {
                for (int dx = -1; dx <= 1; dx++) {
                    ChunkPos unlock = {pos.x + dx, pos.y + dy, pos.z + dz};
                    m_decorationLocks.erase(unlock);
                }
            }
        }
    }
};
```

---

## Pseudo-Code Reference

### Complete Generation Flow

```
FUNCTION updateStreaming(cameraPosition):
    wantedChunks = calculateWantedChunks(cameraPosition)

    FOR EACH pos IN wantedChunks:
        IF NOT chunkExists(pos):
            requestChunk(pos)  // Queues terrain generation

    // Decoration queue is updated by terrainJob callbacks


FUNCTION requestChunk(pos):
    chunk = new Chunk(pos)
    chunk.state = TERRAIN_PENDING
    terrainQueue.enqueue({pos, chunk.generationId})
    chunks[pos] = chunk


FUNCTION terrainWorkerThread():
    WHILE running:
        job = terrainQueue.dequeue()  // Blocks until job available

        chunk = getChunk(job.pos)
        IF chunk == NULL OR chunk.generationId != job.generationId:
            CONTINUE  // Chunk unloaded

        chunk.state = TERRAIN_GENERATING

        // Pass 1: Terrain only
        FOR lx = 0 TO 15:
            FOR lz = 0 TO 15:
                FOR ly = 0 TO 15:
                    worldX = chunk.pos.x * 16 + lx
                    worldY = chunk.pos.y * 16 + ly
                    worldZ = chunk.pos.z * 16 + lz

                    height = getTerrainHeight(worldX, worldZ)
                    block = determineBlock(worldY, height)
                    chunk.setBlock({lx, ly, lz}, block)

        chunk.state = TERRAIN_DONE

        // Check if any chunks can now be decorated
        notifyTerrainComplete(job.pos)


FUNCTION notifyTerrainComplete(completedPos):
    // Check 3x3x3 region centered on completed chunk
    FOR dx = -1 TO 1:
        FOR dy = -1 TO 1:
            FOR dz = -1 TO 1:
                checkPos = completedPos + (dx, dy, dz)
                chunk = getChunk(checkPos)

                IF chunk != NULL AND chunk.state == TERRAIN_DONE:
                    IF canDecorate(checkPos):
                        chunk.state = DECOR_PENDING
                        decorationQueue.enqueue({checkPos, chunk.generationId})


FUNCTION canDecorate(pos):
    // All 27 chunks in 3x3x3 must have terrain
    FOR dx = -1 TO 1:
        FOR dy = -1 TO 1:
            FOR dz = -1 TO 1:
                neighborPos = pos + (dx, dy, dz)
                neighbor = getChunk(neighborPos)

                IF neighbor == NULL:
                    RETURN FALSE
                IF neighbor.state < TERRAIN_DONE:
                    RETURN FALSE

    RETURN TRUE


FUNCTION decorationWorkerThread():
    WHILE running:
        job = decorationQueue.dequeue()

        chunk = getChunk(job.pos)
        IF chunk == NULL OR chunk.generationId != job.generationId:
            CONTINUE

        // Try to acquire decoration lock (prevents concurrent neighbor writes)
        IF NOT tryAcquireDecorationLock(job.pos):
            // Re-queue for later
            decorationQueue.enqueue(job)
            CONTINUE

        chunk.state = DECOR_GENERATING

        // Create neighbor access helper
        neighbors = new NeighborAccess(job.pos, getChunk)

        // Pass 2: Place decorations
        decorateChunk(chunk, neighbors)

        chunk.state = DECOR_DONE
        releaseDecorationLock(job.pos)

        // Queue meshing for affected chunks
        FOR EACH modifiedChunk IN neighbors.getModifiedChunks():
            queueMeshing(modifiedChunk.pos)


FUNCTION decorateChunk(chunk, neighbors):
    // Deterministic random based on chunk position
    seed = hashCombine(worldSeed, chunk.pos.x, chunk.pos.z)
    rng = RandomGenerator(seed)

    // Place trees
    numTrees = rng.nextInt(0, 2)
    FOR i = 0 TO numTrees:
        lx = rng.nextInt(2, 13)  // Avoid chunk edges for tree origins
        lz = rng.nextInt(2, 13)

        worldX = chunk.pos.x * 16 + lx
        worldZ = chunk.pos.z * 16 + lz
        groundY = findGround(neighbors, worldX, worldZ)

        IF neighbors.getBlock(worldX, groundY, worldZ) == GRASS:
            placeTreePrefab(neighbors, worldX, groundY + 1, worldZ)


FUNCTION placeTreePrefab(neighbors, originX, originY, originZ):
    prefab = prefabRegistry.get("oak_tree_1")

    FOR EACH block IN prefab.blocks:
        worldX = originX + block.offsetX
        worldY = originY + block.offsetY
        worldZ = originZ + block.offsetZ

        // NeighborAccess handles cross-chunk writes!
        neighbors.setBlock(worldX, worldY, worldZ, block.material)
```

### NeighborAccess Implementation Detail

```
CLASS NeighborAccess:
    centerPos: ChunkPos
    chunks: Array[27] of Chunk*        // 3x3x3 neighborhood
    modified: Array[27] of Boolean     // Track writes

    CONSTRUCTOR(centerPos, getChunkFunc):
        this.centerPos = centerPos

        // Populate 3x3x3 grid
        FOR dy = -1 TO 1:
            FOR dz = -1 TO 1:
                FOR dx = -1 TO 1:
                    neighborPos = centerPos + (dx, dy, dz)
                    index = (dx+1) + (dy+1)*3 + (dz+1)*9
                    chunks[index] = getChunkFunc(neighborPos)
                    modified[index] = FALSE


    FUNCTION setBlock(worldX, worldY, worldZ, material):
        // Calculate chunk position
        chunkX = worldX >> 4  // Divide by 16
        chunkY = worldY >> 4
        chunkZ = worldZ >> 4

        // Calculate offset from center
        dx = chunkX - centerPos.x
        dy = chunkY - centerPos.y
        dz = chunkZ - centerPos.z

        // Verify within 3x3x3 bounds
        IF dx < -1 OR dx > 1 OR dy < -1 OR dy > 1 OR dz < -1 OR dz > 1:
            RETURN  // Out of range, ignore

        index = (dx+1) + (dy+1)*3 + (dz+1)*9
        chunk = chunks[index]

        IF chunk != NULL AND hasTerrainComplete(chunk.state):
            localX = worldX & 15  // Modulo 16
            localY = worldY & 15
            localZ = worldZ & 15

            chunk.setBlockThreadSafe({localX, localY, localZ}, material)
            modified[index] = TRUE


    FUNCTION getModifiedChunks():
        result = []
        FOR i = 0 TO 26:
            IF modified[i] AND chunks[i] != NULL:
                result.append(chunks[i])
        RETURN result
```

---

## Threading Considerations

### Race Condition Prevention

| Scenario | Risk | Solution |
|----------|------|----------|
| Two decorations write same chunk | Data corruption | Decoration locks or per-chunk mutex |
| Decoration reads while terrain generates | Stale data | State checks before decoration |
| Chunk unloads during decoration | Null pointer | Generation ID verification |
| Meshing reads during decoration | Torn reads | Double buffering (existing) |

### Thread Pool Configuration

```cpp
// Recommended configuration
const int TERRAIN_THREADS = std::thread::hardware_concurrency();
const int DECORATION_THREADS = std::max(1u, std::thread::hardware_concurrency() / 2);

// Decoration is less CPU intensive but has more contention
// Fewer threads = less lock contention
```

### Memory Ordering

For atomic state transitions:

```cpp
// When setting state
chunk->m_state.store(ChunkState::TERRAIN_DONE, std::memory_order_release);

// When reading state
ChunkState state = chunk->m_state.load(std::memory_order_acquire);
```

---

## Edge Cases and Gotchas

### 1. World Edges

At world boundaries, some neighbors don't exist:

```cpp
bool canDecorate(const ChunkPos& pos) const {
    for (int dy = -1; dy <= 1; dy++) {
        for (int dz = -1; dz <= 1; dz++) {
            for (int dx = -1; dx <= 1; dx++) {
                ChunkPos neighborPos = {pos.x + dx, pos.y + dy, pos.z + dz};

                // Option 1: Skip chunks at world edge
                if (isOutsideWorld(neighborPos)) continue;

                // Option 2: Require all neighbors (chunks at edge never decorate)
                // This means no trees at world boundary

                Chunk* neighbor = getChunk(neighborPos);
                if (!neighbor || !hasTerrainComplete(neighbor->getState())) {
                    return false;
                }
            }
        }
    }
    return true;
}
```

### 2. Large Structures

If structures can extend beyond immediate neighbors (>16 blocks):

```cpp
// Option 1: Limit structure size (recommended)
const int MAX_STRUCTURE_RADIUS = 8;  // Stays within 3x3x3

// Option 2: Expand neighborhood check (expensive)
const int NEIGHBOR_RADIUS = 2;  // Check 5x5x5 = 125 chunks
```

### 3. Chunk Unloading During Decoration

```cpp
void decorationJob(ChunkJob& job) {
    // ... acquire lock ...

    NeighborAccess neighbors(job.pos, [this](const ChunkPos& p) {
        return getChunk(p);
    });

    // IMPORTANT: Verify neighbors still exist
    if (!neighbors.allNeighborsReady()) {
        chunk->setState(ChunkState::TERRAIN_DONE);  // Reset state
        releaseDecorationLock(job.pos);
        return;  // Will be re-queued when neighbors load again
    }

    // ... proceed with decoration ...
}
```

### 4. Deterministic Placement

Tree positions must be deterministic regardless of chunk generation order:

```cpp
// WRONG: Different order = different trees
void decorate(Chunk& chunk) {
    srand(time(NULL));  // Non-deterministic!
    int x = rand() % 16;
}

// CORRECT: Same position every time
void decorate(Chunk& chunk) {
    uint32_t seed = hash(worldSeed, chunk.pos.x, chunk.pos.z);
    std::mt19937 rng(seed);  // Deterministic
    int x = rng() % 16;
}
```

### 5. Overlapping Structures

When two chunks both try to place trees that would overlap:

```cpp
void placeTreePrefab(NeighborAccess& neighbors, int x, int y, int z) {
    // Option 1: Check for existing blocks (slower, natural)
    for (const auto& block : prefab.blocks) {
        int wx = x + block.offsetX;
        int wy = y + block.offsetY;
        int wz = z + block.offsetZ;

        Material existing = neighbors.getBlock(wx, wy, wz);
        if (existing.getBlockId() == BlockId::AIR) {
            neighbors.setBlock(wx, wy, wz, block.material);
        }
        // Skip if block already exists (another tree placed it)
    }

    // Option 2: Last write wins (simpler, might look odd)
    // Just place blocks without checking
}
```

---

## Testing Strategy

### Unit Tests

```cpp
// Test 1: NeighborAccess correctly maps world to local coords
TEST(NeighborAccess, WorldToLocalMapping) {
    // World position (18, 5, 33) should map to:
    // Chunk (1, 0, 2), local (2, 5, 1)

    ChunkPos center = {1, 0, 2};
    NeighborAccess access(center, mockGetChunk);

    access.setBlock(18, 5, 33, Material(1));

    EXPECT_EQ(mockChunks[center].getBlock({2, 5, 1}), Material(1));
}

// Test 2: Cross-chunk write works
TEST(NeighborAccess, CrossChunkWrite) {
    ChunkPos center = {0, 0, 0};
    ChunkPos neighbor = {1, 0, 0};

    NeighborAccess access(center, mockGetChunk);

    // World X=16 is in chunk X=1
    access.setBlock(16, 5, 5, Material(1));

    EXPECT_EQ(mockChunks[neighbor].getBlock({0, 5, 5}), Material(1));
}

// Test 3: Decoration waits for neighbors
TEST(ChunkManager, DecorationWaitsForNeighbors) {
    ChunkManager manager;

    // Generate single chunk
    manager.requestChunk({0, 0, 0});
    waitForTerrain();

    Chunk* chunk = manager.getChunk({0, 0, 0});

    // Should still be TERRAIN_DONE (no neighbors)
    EXPECT_EQ(chunk->getState(), ChunkState::TERRAIN_DONE);

    // Generate all neighbors
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dz = -1; dz <= 1; dz++) {
                if (dx == 0 && dy == 0 && dz == 0) continue;
                manager.requestChunk({dx, dy, dz});
            }
        }
    }
    waitForTerrain();
    waitForDecoration();

    // Now should be decorated
    EXPECT_EQ(chunk->getState(), ChunkState::DECOR_DONE);
}
```

### Integration Tests

```cpp
// Test: Trees span chunk boundaries correctly
TEST(TerrainGeneration, TreesSpanChunks) {
    World world(seed);

    // Generate 3x3 chunk area
    for (int x = -1; x <= 1; x++) {
        for (int z = -1; z <= 1; z++) {
            world.requestChunk({x, 0, z});
        }
    }
    world.waitForGeneration();

    // Find a tree that should span chunks
    // (Manually verified tree at known location)

    // Check tree trunk exists in chunk (0,0,0)
    EXPECT_EQ(world.getBlock(14, 65, 8), BlockId::OAK_LOG);

    // Check tree leaves extend into chunk (1,0,0)
    EXPECT_EQ(world.getBlock(16, 67, 8), BlockId::OAK_LEAVES);
    EXPECT_EQ(world.getBlock(17, 67, 8), BlockId::OAK_LEAVES);
}
```

### Visual Tests

1. Generate a flat world with trees
2. Verify no trees are cut off at chunk boundaries
3. Fly around chunk edges to check for visual artifacts

---

## Migration Checklist

- [ ] Update `ChunkState` enum with new states
- [ ] Create `NeighborAccess` class
- [ ] Split `TerrainGenerator::generate()` into `generateTerrain()` and `decorate()`
- [ ] Add decoration eligibility check to `ChunkManager`
- [ ] Implement decoration locking mechanism
- [ ] Update chunk state transitions in `ChunkManager`
- [ ] Add `notifyTerrainComplete()` callback
- [ ] Update meshing triggers to use `DECOR_DONE` state
- [ ] Test cross-chunk tree placement
- [ ] Verify deterministic generation (same seed = same world)
- [ ] Performance test with many chunks loading

---

## Performance Comparison

| Approach | Pros | Cons |
|----------|------|------|
| **Single Pass (current)** | Simple, fast | Structures cut off |
| **Two Pass (this guide)** | Complete structures | Slight delay before decoration |
| **Pending Blocks Queue** | Simpler state machine | Requires synchronized queue |
| **Pre-calculation** | No neighbor dependency | Complex, more computation |

The two-pass approach is used by Minecraft and most professional voxel engines because it provides a good balance of correctness and performance.

---

## References

- Minecraft terrain generation: Uses similar multi-pass approach
- Cuberite voxel engine: Open-source reference implementation
- "GPU Gems 3" Chapter 1: Generating complex procedural terrains
