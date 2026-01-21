# Chunk Render Pipeline - Code Report

This document analyzes the current chunk render pipeline architecture, identifies design flaws, and provides step-by-step guides to address each issue.

---

## Table of Contents

1. [Pipeline Overview](#pipeline-overview)
2. [Design Flaws & Enhancements](#design-flaws--enhancements)
   - [0. CRITICAL: Thread-Unsafe Block Data Access](#0-critical-thread-unsafe-block-data-access)
   - [1. No Frustum Culling](#1-no-frustum-culling)
   - [2. No Greedy Meshing](#2-no-greedy-meshing)
   - [3. Vertex Data Overhead](#3-vertex-data-overhead)
   - [4. One Draw Call Per Chunk](#4-one-draw-call-per-chunk)
   - [5. Unused Dirty Flag](#5-unused-dirty-flag)
   - [6. Neighbor Dependency on Meshing](#6-neighbor-dependency-on-meshing)
   - [7. No Level of Detail (LOD)](#7-no-level-of-detail-lod)
   - [8. Upload Queue Bottleneck](#8-upload-queue-bottleneck)
   - [9. No Transparent Block Handling](#9-no-transparent-block-handling)
   - [10. VAO/VBO Fragmentation](#10-vaovbo-fragmentation)

---

## Pipeline Overview

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           CHUNK RENDER PIPELINE                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐    ┌───────────┐ │
│  │   UNLOADED   │───▶│  GENERATING  │───▶│  GENERATED   │───▶│  MESHING  │ │
│  └──────────────┘    └──────────────┘    └──────────────┘    └───────────┘ │
│                       (ThreadPool 1)                          (ThreadPool 2)│
│                                                                      │      │
│                                                                      ▼      │
│  ┌──────────────┐    ┌──────────────┐                        ┌───────────┐ │
│  │   RENDERED   │◀───│    READY     │◀───────────────────────│   MESHED  │ │
│  └──────────────┘    └──────────────┘                        └───────────┘ │
│   (Main Thread)       (GPU Upload)                                          │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

**Current Implementation:**
- `ChunkManager` handles chunk lifecycle and terrain generation
- `ChunkMeshManager` handles mesh building and GPU uploads
- Two separate `ThreadPool` instances for generation and meshing
- Face culling against air blocks reduces triangle count
- One VAO/VBO per chunk, one draw call per chunk

---

## Design Flaws & Enhancements

---

### 0. CRITICAL: Thread-Unsafe Block Data Access

**Severity: CRITICAL - Can cause crashes, data corruption, undefined behavior**

**Problem:**
The `Chunk::blocks` array (`std::array<Material, 4096>`) has **no synchronization** and is accessed concurrently by multiple threads:

```cpp
// Chunk.h - NO PROTECTION!
std::array<Material, VOLUME> blocks{};  // Plain array, not thread-safe
```

While `state`, `dirty`, and `generationID` are atomic, the actual block data is not protected.

**Thread Access Analysis:**

| Thread | Operation | Method | Access Type |
|--------|-----------|--------|-------------|
| Main Thread | Block placement | `World::setBlock()` → `Chunk::setBlock()` | **WRITE** |
| Main Thread | Block query | `World::getBlock()` → `Chunk::getBlock()` | READ |
| Main Thread | Raycast | `Camera::raycast()` → `World::isAir()` | READ |
| Generation Worker | Terrain gen | `TerrainGenerator::generate()` → `Chunk::setBlock()` | **WRITE** |
| Meshing Worker | Mesh build | `buildMeshJob()` → `Chunk::getBlock()`, `isAir()` | READ |
| Meshing Worker | Neighbor access | `isAirAt()` → `neighbor->isAir()` | READ |

**Race Conditions Identified:**

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         DATA RACE SCENARIOS                                 │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  RACE 1: Main Thread Write vs Meshing Worker Read                          │
│  ─────────────────────────────────────────────────                          │
│  Main Thread                    Meshing Worker                              │
│  ────────────                   ──────────────                              │
│  World::setBlock(x,y,z, STONE)                                              │
│    │                            buildMeshJob()                              │
│    ▼                              │                                         │
│  blocks[i] = STONE  ←─ RACE! ─→  blocks[i] read                            │
│                                                                             │
│  RACE 2: Generation Worker Write vs Meshing Worker Read                    │
│  ──────────────────────────────────────────────────────                     │
│  After state = GENERATED, meshing can start immediately                    │
│  But generation might still be finishing writes (compiler reordering)      │
│                                                                             │
│  RACE 3: Chunk Map Modification vs Worker Pointer Access                   │
│  ───────────────────────────────────────────────────────                    │
│  Main Thread                    Worker Thread                               │
│  ────────────                   ─────────────                               │
│  updateStreaming()              buildMeshJob()                              │
│    │                              │                                         │
│    ▼                              ▼                                         │
│  chunks.erase(pos) ←─ RACE! ─→  chunk->getBlock()                          │
│  (Chunk destroyed)              (Dangling pointer!)                         │
│                                                                             │
│  RACE 4: Neighbor Chunk Access                                              │
│  ────────────────────────────────                                           │
│  Main Thread                    Worker Thread                               │
│  ────────────                   ─────────────                               │
│  chunks.erase(neighborPos)      isAirAt() calls n.west->isAir()            │
│  (Neighbor destroyed)           (Dangling pointer to neighbor!)            │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

**Locations:**

| File | Line | Issue |
|------|------|-------|
| `Chunk.h:45` | `std::array<Material, VOLUME> blocks{}` | Unprotected shared data |
| `Chunk.cpp:34-38` | `setBlock()` | Unsynchronized write |
| `Chunk.cpp:24-27` | `getBlock()` | Unsynchronized read |
| `ChunkManager.cpp:92` | `chunks.erase(it)` | Destroys chunk while workers may hold pointers |
| `ChunkMeshManager.cpp:70` | `getChunk()` | Gets raw pointer that can become dangling |
| `ChunkMeshManager.cpp:78` | `getNeighbors()` | Gets neighbor pointers that can become dangling |
| `ChunkMeshManager.cpp:194-199` | `isAirAt()` | Accesses neighbor chunk data |

**Symptoms (may occur randomly):**

- Corrupted mesh geometry (torn faces, wrong textures)
- Crashes in `buildMeshJob()` (access violation)
- Crashes when unloading chunks (use-after-free)
- Inconsistent block states
- Hard-to-reproduce bugs that only appear under load

**Fix - Step by Step:**

There are multiple approaches, from simplest to most performant:

---

#### Option A: Reader-Writer Lock per Chunk (Simplest)

1. **Add a shared mutex to Chunk:**

```cpp
// Chunk.h
#include <shared_mutex>

class Chunk {
private:
    mutable std::shared_mutex m_blocksMutex;
    std::array<Material, VOLUME> blocks{};
    // ...
};
```

2. **Protect reads with shared lock:**

```cpp
// Chunk.cpp
Material Chunk::getBlock(uint8_t x, uint8_t y, uint8_t z) const {
    std::shared_lock lock(m_blocksMutex);  // Multiple readers OK
    return blocks[ChunkCoords::localCoordsToIndex(x, y, z)];
}

bool Chunk::isAir(uint8_t x, uint8_t y, uint8_t z) const {
    std::shared_lock lock(m_blocksMutex);
    return blocks[ChunkCoords::localCoordsToIndex(x, y, z)] == 0;
}
```

3. **Protect writes with exclusive lock:**

```cpp
void Chunk::setBlock(uint8_t x, uint8_t y, uint8_t z, Material id) {
    std::unique_lock lock(m_blocksMutex);  // Exclusive access
    blocks[ChunkCoords::localCoordsToIndex(x, y, z)] = id;
    // Note: setState() uses its own atomic, no need to hold lock
}
```

4. **Add bulk read for meshing (reduces lock contention):**

```cpp
// Chunk.h
std::array<Material, VOLUME> getBlocksCopy() const;

// Chunk.cpp
std::array<Material, VOLUME> Chunk::getBlocksCopy() const {
    std::shared_lock lock(m_blocksMutex);
    return blocks;  // Copy entire array under lock
}
```

5. **Update buildMeshJob to use snapshot:**

```cpp
void ChunkMeshManager::buildMeshJob(const ChunkJob& job) {
    Chunk* chunk = world.getChunkManager()->getChunk(job.pos);
    if (!chunk) return;

    // Take a snapshot of block data - no further locking needed
    auto blockData = chunk->getBlocksCopy();

    // Also snapshot neighbor data
    auto neighbors = world.getChunkManager()->getNeighbors(job.pos);
    std::array<std::optional<std::array<Material, Chunk::VOLUME>>, 6> neighborData;

    if (neighbors.north) neighborData[0] = neighbors.north->getBlocksCopy();
    if (neighbors.south) neighborData[1] = neighbors.south->getBlocksCopy();
    // ... etc

    // Now build mesh using local copies - completely thread-safe
    // ...
}
```

---

#### Option B: Copy-on-Read with Shared Pointers (Safer)

1. **Use shared_ptr for chunk storage:**

```cpp
// ChunkManager.h
using ChunkPtr = std::shared_ptr<Chunk>;
using ChunkMap = std::unordered_map<ChunkPos, ChunkPtr, ChunkPosHash>;
```

2. **Workers hold shared_ptr, not raw pointer:**

```cpp
struct ChunkJob {
    ChunkPos pos;
    float distance;
    uint64_t generationID;
    std::shared_ptr<Chunk> chunkRef;  // Prevents destruction while job runs
};
```

3. **Enqueue with shared ownership:**

```cpp
void ChunkMeshManager::scheduleMeshing(const glm::vec3& cameraPos) {
    for (auto& [pos, chunkPtr] : world.getChunkManager()->getChunks()) {
        if (chunkPtr->getState() != ChunkState::GENERATED)
            continue;

        workers.enqueue({
            pos,
            distance,
            chunkPtr->getGenerationID(),
            chunkPtr  // Share ownership with worker
        });
    }
}
```

4. **Chunk survives until job completes:**

```cpp
void ChunkMeshManager::buildMeshJob(const ChunkJob& job) {
    // job.chunkRef keeps chunk alive even if erased from map
    Chunk& chunk = *job.chunkRef;

    if (chunk.getGenerationID() != job.generationID)
        return;  // Stale job, but no crash!

    // Safe to access chunk data
    // ...
}
```

---

#### Option C: State Machine Enforcement (Most Correct)

1. **Define exclusive states that prevent concurrent access:**

```cpp
enum class ChunkState {
    UNLOADED,           // No data
    GENERATING,         // Generation worker has exclusive write access
    GENERATED,          // Immutable until state changes
    PENDING_MESHING,    // Waiting for meshing (still immutable)
    MESHING,            // Meshing worker has read access
    MESHED,             // Waiting for upload
    READY,              // Renderable, main thread can modify
    PENDING_REMESH      // Main thread modified, needs remesh
};
```

2. **Enforce state transitions:**

```cpp
bool Chunk::tryTransitionTo(ChunkState from, ChunkState to) {
    return state.compare_exchange_strong(from, to);
}

// Only transition GENERATED → MESHING if not already meshing
if (!chunk.tryTransitionTo(ChunkState::GENERATED, ChunkState::MESHING))
    return;  // Someone else got it first
```

3. **Block modifications only in READY state:**

```cpp
void World::setBlock(int wx, int wy, int wz, Material id) {
    Chunk* chunk = getChunk(wx, wy, wz);

    // Wait or fail if chunk is being processed
    if (chunk->getState() != ChunkState::READY) {
        return;  // Or queue the modification for later
    }

    chunk->setBlock(x, y, z, id);
    chunk->setState(ChunkState::PENDING_REMESH);
}
```

4. **Add memory barriers for state transitions:**

```cpp
void Chunk::setState(ChunkState newState) {
    // Ensure all prior writes are visible before state change
    std::atomic_thread_fence(std::memory_order_release);
    state.store(newState, std::memory_order_release);
}

ChunkState Chunk::getState() const {
    ChunkState s = state.load(std::memory_order_acquire);
    std::atomic_thread_fence(std::memory_order_acquire);
    return s;
}
```

---

#### Option D: Double Buffering (Most Performant)

1. **Two block arrays per chunk:**

```cpp
class Chunk {
    std::array<Material, VOLUME> m_blocks[2];
    std::atomic<int> m_readIndex{0};  // Which buffer workers read from
    // Main thread always writes to !m_readIndex
};
```

2. **Workers read from stable buffer:**

```cpp
const std::array<Material, VOLUME>& Chunk::getReadBuffer() const {
    return m_blocks[m_readIndex.load(std::memory_order_acquire)];
}
```

3. **Main thread writes to back buffer, then swaps:**

```cpp
void Chunk::setBlock(uint8_t x, uint8_t y, uint8_t z, Material id) {
    int writeIndex = 1 - m_readIndex.load();
    m_blocks[writeIndex][index] = id;
}

void Chunk::commitChanges() {
    int oldRead = m_readIndex.load();
    int newRead = 1 - oldRead;

    // Copy modified buffer to the other
    m_blocks[oldRead] = m_blocks[newRead];

    // Swap (workers now see new data)
    m_readIndex.store(newRead, std::memory_order_release);
}
```

---

**Recommended Approach:**

For your codebase, **Option A (Reader-Writer Lock) + Option B (Shared Pointers)** combined:

1. Use `std::shared_ptr<Chunk>` to prevent use-after-free
2. Use `std::shared_mutex` to protect block data
3. Use `getBlocksCopy()` in meshing to minimize lock hold time

**Minimal Fix (immediate):**

```cpp
// Chunk.h - Add these
#include <shared_mutex>

class Chunk {
private:
    mutable std::shared_mutex m_blocksMutex;
    // ... existing members

public:
    // Add bulk accessor for meshing
    std::array<Material, VOLUME> getBlockSnapshot() const {
        std::shared_lock lock(m_blocksMutex);
        return blocks;
    }
};

// Chunk.cpp - Modify existing methods
Material Chunk::getBlock(uint8_t x, uint8_t y, uint8_t z) const {
    std::shared_lock lock(m_blocksMutex);
    return blocks[ChunkCoords::localCoordsToIndex(x, y, z)];
}

void Chunk::setBlock(uint8_t x, uint8_t y, uint8_t z, Material id) {
    {
        std::unique_lock lock(m_blocksMutex);
        blocks[ChunkCoords::localCoordsToIndex(x, y, z)] = id;
    }
    setState(ChunkState::GENERATED);
}
```

**For the dangling pointer issue, additionally:**

```cpp
// ChunkManager.h
using ChunkPtr = std::shared_ptr<Chunk>;
std::unordered_map<ChunkPos, ChunkPtr, ChunkPosHash> chunks;

// ChunkJob struct
struct ChunkJob {
    ChunkPos pos;
    float distance;
    uint64_t generationID;
    std::weak_ptr<Chunk> chunkRef;  // Weak ref to detect if chunk was unloaded
};

// In worker
void buildMeshJob(const ChunkJob& job) {
    auto chunk = job.chunkRef.lock();
    if (!chunk) return;  // Chunk was unloaded, abort safely
    // ...
}
```

---

### 1. No Frustum Culling

**Problem:**
`World::render()` iterates over all READY chunks and renders them regardless of visibility. Chunks behind the camera or outside the view frustum are still sent to the GPU.

**Impact:**
- Wasted draw calls (up to 70% of chunks may be off-screen)
- GPU processes vertices that get clipped anyway
- Reduced frame rate with large view distances

**Location:** `src/Content/World/World.cpp` - `render()` method

**Fix - Step by Step:**

1. **Create a Frustum class** (`src/Engine/Render/Frustum/Frustum.h`):

```cpp
class Frustum {
public:
    void update(const glm::mat4& viewProjection);
    bool isBoxVisible(const glm::vec3& min, const glm::vec3& max) const;

private:
    // 6 planes: left, right, bottom, top, near, far
    std::array<glm::vec4, 6> m_planes;
};
```

2. **Extract frustum planes from VP matrix:**

```cpp
void Frustum::update(const glm::mat4& vp) {
    // Left plane
    m_planes[0] = glm::vec4(
        vp[0][3] + vp[0][0],
        vp[1][3] + vp[1][0],
        vp[2][3] + vp[2][0],
        vp[3][3] + vp[3][0]
    );
    // Right plane
    m_planes[1] = glm::vec4(
        vp[0][3] - vp[0][0],
        vp[1][3] - vp[1][0],
        vp[2][3] - vp[2][0],
        vp[3][3] - vp[3][0]
    );
    // ... bottom, top, near, far (similar pattern)

    // Normalize all planes
    for (auto& plane : m_planes) {
        float length = glm::length(glm::vec3(plane));
        plane /= length;
    }
}
```

3. **AABB-Frustum intersection test:**

```cpp
bool Frustum::isBoxVisible(const glm::vec3& min, const glm::vec3& max) const {
    for (const auto& plane : m_planes) {
        glm::vec3 positive = min;
        if (plane.x >= 0) positive.x = max.x;
        if (plane.y >= 0) positive.y = max.y;
        if (plane.z >= 0) positive.z = max.z;

        if (glm::dot(glm::vec3(plane), positive) + plane.w < 0)
            return false;
    }
    return true;
}
```

4. **Integrate into World::render():**

```cpp
void World::render(const Frustum& frustum) {
    for (auto& [pos, mesh] : m_meshManager.getMeshes()) {
        glm::vec3 min = glm::vec3(pos.x * 16, pos.y * 16, pos.z * 16);
        glm::vec3 max = min + glm::vec3(16.0f);

        if (!frustum.isBoxVisible(min, max))
            continue;  // Skip this chunk

        // ... render chunk
    }
}
```

5. **Update frustum each frame in Engine:**

```cpp
void Engine::render() {
    glm::mat4 vp = m_projection * m_player.getCamera().getViewMatrix();
    m_frustum.update(vp);
    m_world.render(m_frustum);
}
```

---

### 2. No Greedy Meshing

**Problem:**
Each visible block face generates 2 triangles (6 vertices). Adjacent faces of the same texture are not merged, resulting in excessive vertex count.

**Impact:**
- 10-50x more vertices than necessary
- Higher memory usage
- More GPU vertex processing

**Location:** `src/Content/ChunkMeshManager/ChunkMeshManager.cpp` - `buildFaceMesh()`

**Current approach:**
```
┌───┬───┬───┬───┐
│ 2▲│ 2▲│ 2▲│ 2▲│  = 8 triangles (24 vertices)
└───┴───┴───┴───┘
```

**Greedy approach:**
```
┌───────────────┐
│      2▲       │  = 2 triangles (6 vertices)
└───────────────┘
```

**Fix - Step by Step:**

1. **Create a 2D mask for each face direction:**

```cpp
struct FaceMask {
    Material material;
    bool culled;
};

// For each slice perpendicular to the face normal
std::array<FaceMask, 16 * 16> mask;
```

2. **Fill the mask for one slice:**

```cpp
void fillMask(const Chunk& chunk, Face face, int depth,
              std::array<FaceMask, 256>& mask) {
    for (int u = 0; u < 16; u++) {
        for (int v = 0; v < 16; v++) {
            glm::ivec3 pos = sliceToBlockPos(face, depth, u, v);
            Material mat = chunk.getBlock(pos);
            bool visible = mat != Material::AIR && isAirAdjacent(chunk, pos, face);
            mask[u + v * 16] = { mat, !visible };
        }
    }
}
```

3. **Greedy merge algorithm:**

```cpp
void greedyMerge(std::array<FaceMask, 256>& mask,
                 std::vector<Quad>& quads) {
    for (int j = 0; j < 16; j++) {
        for (int i = 0; i < 16; ) {
            FaceMask& current = mask[i + j * 16];
            if (current.culled) { i++; continue; }

            // Expand width
            int width = 1;
            while (i + width < 16 && canMerge(current, mask[i + width + j * 16]))
                width++;

            // Expand height
            int height = 1;
            bool canExpandHeight = true;
            while (j + height < 16 && canExpandHeight) {
                for (int k = 0; k < width; k++) {
                    if (!canMerge(current, mask[i + k + (j + height) * 16])) {
                        canExpandHeight = false;
                        break;
                    }
                }
                if (canExpandHeight) height++;
            }

            // Emit quad
            quads.push_back({ i, j, width, height, current.material });

            // Mark merged cells as culled
            for (int dy = 0; dy < height; dy++)
                for (int dx = 0; dx < width; dx++)
                    mask[i + dx + (j + dy) * 16].culled = true;

            i += width;
        }
    }
}
```

4. **Generate vertices from quads:**

```cpp
void emitQuad(const Quad& quad, Face face, int depth,
              std::vector<Vertex>& vertices) {
    glm::ivec3 v0 = quadToWorldPos(face, depth, quad.x, quad.y);
    glm::ivec3 v1 = quadToWorldPos(face, depth, quad.x + quad.w, quad.y);
    glm::ivec3 v2 = quadToWorldPos(face, depth, quad.x + quad.w, quad.y + quad.h);
    glm::ivec3 v3 = quadToWorldPos(face, depth, quad.x, quad.y + quad.h);

    // UVs scaled by quad dimensions for tiling
    // Triangle 1: v0, v1, v2
    // Triangle 2: v0, v2, v3
}
```

5. **Process all 6 face directions:**

```cpp
MeshData buildGreedyMesh(const Chunk& chunk, const ChunkNeighbors& neighbors) {
    std::vector<Vertex> vertices;

    for (Face face : { NORTH, SOUTH, EAST, WEST, TOP, BOTTOM }) {
        for (int depth = 0; depth < 16; depth++) {
            std::array<FaceMask, 256> mask;
            fillMask(chunk, face, depth, mask);

            std::vector<Quad> quads;
            greedyMerge(mask, quads);

            for (const Quad& quad : quads)
                emitQuad(quad, face, depth, vertices);
        }
    }

    return MeshData{ std::move(vertices) };
}
```

---

### 3. Vertex Data Overhead

**Problem:**
Current vertex structure uses 40 bytes per vertex:
```cpp
struct Vertex {
    glm::ivec3 position;   // 12 bytes
    glm::ivec3 normal;     // 12 bytes (only 6 possible values!)
    glm::ivec2 uv;         // 8 bytes (only 0 or 1!)
    unsigned int texId;    // 4 bytes
};                         // Total: 36 bytes + padding = 40 bytes
```

**Impact:**
- High memory bandwidth consumption
- Larger GPU buffer allocations
- Cache inefficiency

**Location:** `src/Content/ChunkMesh/ChunkMesh.h`

**Fix - Step by Step:**

1. **Pack vertex data into 8 bytes:**

```cpp
struct PackedVertex {
    uint32_t positionAndNormal;  // x:5, y:5, z:5, normalIndex:3, unused:14
    uint32_t uvAndTexture;       // u:5, v:5, texId:16, unused:6
};
```

2. **Encoding function:**

```cpp
PackedVertex packVertex(glm::ivec3 pos, int normalIndex,
                        glm::ivec2 uv, uint16_t texId) {
    uint32_t packed1 =
        (pos.x & 0x1F) |
        ((pos.y & 0x1F) << 5) |
        ((pos.z & 0x1F) << 10) |
        ((normalIndex & 0x7) << 15);

    uint32_t packed2 =
        (uv.x & 0x1F) |
        ((uv.y & 0x1F) << 5) |
        ((texId & 0xFFFF) << 10);

    return { packed1, packed2 };
}
```

3. **Update VAO attribute setup:**

```cpp
void VAO::storePackedBlockData(const std::vector<PackedVertex>& data) {
    bind();
    m_vbo.addData(data);

    // Single packed attribute
    glVertexAttribIPointer(0, 2, GL_UNSIGNED_INT, sizeof(PackedVertex), (void*)0);
    glEnableVertexAttribArray(0);

    unbind();
}
```

4. **Update vertex shader to unpack:**

```glsl
layout (location = 0) in uvec2 packedData;

const vec3 NORMALS[6] = vec3[](
    vec3( 0,  0, -1),  // NORTH
    vec3( 0,  0,  1),  // SOUTH
    vec3( 1,  0,  0),  // EAST
    vec3(-1,  0,  0),  // WEST
    vec3( 0,  1,  0),  // TOP
    vec3( 0, -1,  0)   // BOTTOM
);

void main() {
    // Unpack position
    vec3 position = vec3(
        float(packedData.x & 0x1Fu),
        float((packedData.x >> 5) & 0x1Fu),
        float((packedData.x >> 10) & 0x1Fu)
    );

    // Unpack normal
    uint normalIndex = (packedData.x >> 15) & 0x7u;
    vec3 normal = NORMALS[normalIndex];

    // Unpack UV and texture
    vec2 uv = vec2(
        float(packedData.y & 0x1Fu),
        float((packedData.y >> 5) & 0x1Fu)
    );
    uint texId = (packedData.y >> 10) & 0xFFFFu;

    // ... rest of shader
}
```

**Result:** 40 bytes → 8 bytes per vertex (80% reduction)

---

### 4. One Draw Call Per Chunk

**Problem:**
Each chunk issues a separate `glDrawArrays()` call. With VIEW_DISTANCE=8, this means ~200+ draw calls per frame.

**Impact:**
- High CPU overhead from draw call submission
- GPU state changes between draws
- Driver overhead

**Location:** `src/Content/World/World.cpp` - render loop

**Fix - Step by Step (Indirect Drawing):**

1. **Create a single large VBO for all chunks:**

```cpp
class WorldMeshBuffer {
    GLuint m_vbo;
    GLuint m_vao;
    size_t m_capacity;
    size_t m_used;

    // Track where each chunk's data lives
    std::unordered_map<ChunkPos, BufferRegion> m_regions;
};

struct BufferRegion {
    size_t offset;      // Byte offset in buffer
    size_t vertexCount;
};
```

2. **Allocate regions for chunks:**

```cpp
BufferRegion WorldMeshBuffer::allocate(ChunkPos pos, size_t vertexCount) {
    // Simple bump allocator (or use free list for reuse)
    BufferRegion region = { m_used, vertexCount };
    m_used += vertexCount * sizeof(PackedVertex);
    m_regions[pos] = region;
    return region;
}

void WorldMeshBuffer::upload(ChunkPos pos, const std::vector<PackedVertex>& data) {
    BufferRegion& region = m_regions[pos];
    glNamedBufferSubData(m_vbo, region.offset,
                         data.size() * sizeof(PackedVertex), data.data());
}
```

3. **Use indirect draw commands:**

```cpp
struct DrawArraysIndirectCommand {
    GLuint count;       // Vertex count
    GLuint instanceCount;
    GLuint first;       // First vertex index
    GLuint baseInstance;
};

class WorldMeshBuffer {
    GLuint m_indirectBuffer;
    std::vector<DrawArraysIndirectCommand> m_commands;
};
```

4. **Build indirect command buffer:**

```cpp
void WorldMeshBuffer::buildDrawCommands(const Frustum& frustum) {
    m_commands.clear();

    for (auto& [pos, region] : m_regions) {
        if (!frustum.isChunkVisible(pos))
            continue;

        m_commands.push_back({
            .count = static_cast<GLuint>(region.vertexCount),
            .instanceCount = 1,
            .first = static_cast<GLuint>(region.offset / sizeof(PackedVertex)),
            .baseInstance = 0
        });
    }

    glNamedBufferSubData(m_indirectBuffer, 0,
                         m_commands.size() * sizeof(DrawArraysIndirectCommand),
                         m_commands.data());
}
```

5. **Single draw call for all chunks:**

```cpp
void WorldMeshBuffer::render() {
    glBindVertexArray(m_vao);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectBuffer);
    glMultiDrawArraysIndirect(GL_TRIANGLES, nullptr, m_commands.size(), 0);
}
```

**Alternative (simpler): Batch nearby chunks:**

```cpp
void World::render() {
    // Group chunks by region, merge their vertex data
    // Reduces draw calls from 200+ to ~20-30
}
```

---

### 5. Unused Dirty Flag

**Problem:**
`Chunk` has an atomic `dirty` flag but it's never used. When a block changes, there's no mechanism to trigger remeshing.

**Impact:**
- Block modifications don't update visuals (if implemented)
- No incremental mesh updates
- Would require full chunk reload for changes

**Location:** `src/Content/Chunk/Chunk.h` - `m_dirty` member

**Fix - Step by Step:**

1. **Set dirty flag when block changes:**

```cpp
void Chunk::setBlock(const glm::ivec3& pos, Material mat) {
    m_blocks[Utils::coordsToIndex(pos)] = mat;
    m_dirty.store(true, std::memory_order_release);

    // Also mark neighbors dirty if block is on edge
    if (pos.x == 0)  notifyNeighborDirty(WEST);
    if (pos.x == 15) notifyNeighborDirty(EAST);
    // ... etc for all 6 faces
}
```

2. **Check dirty flag in mesh scheduler:**

```cpp
void ChunkMeshManager::scheduleMeshing(const glm::vec3& cameraPos) {
    for (auto& [pos, chunk] : m_chunkManager.getChunks()) {
        bool needsMesh =
            chunk.getState() == ChunkState::GENERATED ||
            (chunk.getState() == ChunkState::READY && chunk.isDirty());

        if (needsMesh) {
            chunk.clearDirty();
            chunk.setState(ChunkState::MESHING);
            m_threadPool.enqueue(ChunkJob{ pos, distance, chunk.getGenerationID() });
        }
    }
}
```

3. **Clear dirty flag after mesh upload:**

```cpp
void ChunkMeshManager::update() {
    while (!uploadQueue.empty()) {
        auto [pos, data] = std::move(uploadQueue.front());
        uploadQueue.pop();

        meshes[pos].upload(std::move(data));
        chunk->clearDirty();
        chunk->setState(ChunkState::READY);
    }
}
```

4. **Add helper methods to Chunk:**

```cpp
class Chunk {
public:
    bool isDirty() const {
        return m_dirty.load(std::memory_order_acquire);
    }
    void setDirty() {
        m_dirty.store(true, std::memory_order_release);
    }
    void clearDirty() {
        m_dirty.store(false, std::memory_order_release);
    }
};
```

---

### 6. Neighbor Dependency on Meshing

**Problem:**
Mesh building requires neighbor chunks for proper face culling at boundaries. If a neighbor doesn't exist, faces are generated (assumes air). When the neighbor loads, the mesh isn't rebuilt.

**Impact:**
- Visual artifacts at chunk boundaries
- Holes appear then persist even after neighbors load
- Inconsistent culling behavior

**Location:** `src/Content/ChunkMeshManager/ChunkMeshManager.cpp` - `isAirAt()`

**Fix - Step by Step:**

1. **Track mesh dependencies:**

```cpp
struct MeshDependency {
    std::bitset<6> hasNeighbor;  // Which neighbors existed during mesh build
};

std::unordered_map<ChunkPos, MeshDependency> m_meshDependencies;
```

2. **Record dependencies during mesh build:**

```cpp
MeshData ChunkMeshManager::buildMeshJob(const Chunk& chunk,
                                         const ChunkNeighbors& neighbors) {
    MeshDependency dep;
    dep.hasNeighbor[NORTH] = neighbors.north != nullptr;
    dep.hasNeighbor[SOUTH] = neighbors.south != nullptr;
    // ... etc

    // Store with mesh data for later check
    return MeshData{ vertices, dep };
}
```

3. **Check for invalidation when neighbors load:**

```cpp
void ChunkManager::onChunkGenerated(ChunkPos pos) {
    // Mark this chunk ready
    getChunk(pos)->setState(ChunkState::GENERATED);

    // Check if any neighbor needs remeshing
    for (auto neighborPos : getAdjacentPositions(pos)) {
        if (auto* neighbor = getChunk(neighborPos)) {
            if (neighbor->getState() == ChunkState::READY) {
                auto& dep = m_meshDependencies[neighborPos];
                int faceIndex = getFaceToward(neighborPos, pos);

                if (!dep.hasNeighbor[faceIndex]) {
                    // Neighbor was meshed without us - remesh it
                    neighbor->setDirty();
                }
            }
        }
    }
}
```

4. **Alternative: Require all neighbors before meshing:**

```cpp
bool ChunkMeshManager::canMesh(ChunkPos pos) {
    for (auto neighborPos : getAdjacentPositions(pos)) {
        Chunk* neighbor = m_chunkManager.getChunk(neighborPos);
        if (!neighbor || neighbor->getState() < ChunkState::GENERATED)
            return false;
    }
    return true;
}
```

This delays meshing but ensures correct results. Combine with priority boosting for chunks with all neighbors ready.

---

### 7. No Level of Detail (LOD)

**Problem:**
All chunks use the same mesh detail regardless of distance. Far chunks could use simplified meshes.

**Impact:**
- Excessive vertex count for distant chunks
- Reduced draw distance capability
- Wasted GPU resources

**Fix - Step by Step:**

1. **Define LOD levels:**

```cpp
enum class LODLevel {
    FULL,       // Normal mesh (distance < 32)
    HALF,       // Skip every other block (distance < 64)
    QUARTER,    // Skip 3/4 blocks (distance < 128)
    BILLBOARD   // Single textured quad (distance >= 128)
};

LODLevel getLODLevel(float distance) {
    if (distance < 32.0f)  return LODLevel::FULL;
    if (distance < 64.0f)  return LODLevel::HALF;
    if (distance < 128.0f) return LODLevel::QUARTER;
    return LODLevel::BILLBOARD;
}
```

2. **Store multiple LOD meshes per chunk:**

```cpp
class ChunkMesh {
    std::array<VAO, 4> m_lods;
    std::array<size_t, 4> m_vertexCounts;

public:
    void render(LODLevel lod) {
        int index = static_cast<int>(lod);
        m_lods[index].bind();
        glDrawArrays(GL_TRIANGLES, 0, m_vertexCounts[index]);
    }
};
```

3. **Build LOD meshes with block skipping:**

```cpp
MeshData buildMeshLOD(const Chunk& chunk, LODLevel lod) {
    int step = 1 << static_cast<int>(lod);  // 1, 2, 4, 8

    for (int x = 0; x < 16; x += step) {
        for (int y = 0; y < 16; y += step) {
            for (int z = 0; z < 16; z += step) {
                // Sample block at this position
                // Generate faces scaled by step
            }
        }
    }
}
```

4. **Select LOD at render time:**

```cpp
void World::render(const Frustum& frustum, const glm::vec3& cameraPos) {
    for (auto& [pos, mesh] : m_meshes) {
        float distance = glm::distance(cameraPos, getChunkCenter(pos));
        LODLevel lod = getLODLevel(distance);

        mesh.render(lod);
    }
}
```

5. **Lazy LOD generation:**

```cpp
void ChunkMesh::ensureLOD(LODLevel lod, const Chunk& chunk) {
    int index = static_cast<int>(lod);
    if (m_vertexCounts[index] == 0) {
        // Build LOD mesh on demand
        MeshData data = buildMeshLOD(chunk, lod);
        m_lods[index].upload(data);
        m_vertexCounts[index] = data.vertices.size();
    }
}
```

---

### 8. Upload Queue Bottleneck

**Problem:**
All mesh uploads happen on the main thread in `ChunkMeshManager::update()`. Processing many uploads can cause frame drops.

**Impact:**
- Frame stuttering when many chunks load simultaneously
- Initial world load causes lag spikes
- Camera movement into new areas causes hitches

**Location:** `src/Content/ChunkMeshManager/ChunkMeshManager.cpp` - `update()`

**Fix - Step by Step:**

1. **Limit uploads per frame:**

```cpp
void ChunkMeshManager::update() {
    constexpr int MAX_UPLOADS_PER_FRAME = 4;
    int uploads = 0;

    std::lock_guard lock(m_uploadMutex);
    while (!m_uploadQueue.empty() && uploads < MAX_UPLOADS_PER_FRAME) {
        auto [pos, data] = std::move(m_uploadQueue.front());
        m_uploadQueue.pop();

        m_meshes[pos].upload(std::move(data));
        uploads++;
    }
}
```

2. **Use Persistent Mapped Buffers (advanced):**

```cpp
class PersistentBuffer {
    GLuint m_buffer;
    void* m_mappedPtr;
    size_t m_size;

public:
    void create(size_t size) {
        glCreateBuffers(1, &m_buffer);

        GLbitfield flags = GL_MAP_WRITE_BIT |
                          GL_MAP_PERSISTENT_BIT |
                          GL_MAP_COHERENT_BIT;

        glNamedBufferStorage(m_buffer, size, nullptr, flags);
        m_mappedPtr = glMapNamedBufferRange(m_buffer, 0, size, flags);
    }

    void write(size_t offset, const void* data, size_t size) {
        // Can be called from any thread!
        memcpy((char*)m_mappedPtr + offset, data, size);
    }
};
```

3. **Triple buffering to avoid stalls:**

```cpp
class TripleBufferedUploader {
    std::array<PersistentBuffer, 3> m_buffers;
    std::array<GLsync, 3> m_fences;
    int m_writeIndex = 0;
    int m_readIndex = 0;

    void upload(const MeshData& data) {
        // Wait for buffer to be available
        if (m_fences[m_writeIndex]) {
            glClientWaitSync(m_fences[m_writeIndex], 0, GL_TIMEOUT_IGNORED);
            glDeleteSync(m_fences[m_writeIndex]);
        }

        // Write to buffer (can be threaded)
        m_buffers[m_writeIndex].write(0, data.data(), data.size());

        // Copy to actual mesh buffer
        glCopyNamedBufferSubData(
            m_buffers[m_writeIndex].id(),
            m_meshBuffer.id(),
            0, offset, data.size()
        );

        // Fence for next use
        m_fences[m_writeIndex] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        m_writeIndex = (m_writeIndex + 1) % 3;
    }
};
```

---

### 9. No Transparent Block Handling

**Problem:**
Face culling checks `isAir()` but doesn't handle transparent blocks (glass, water, leaves). Transparent blocks adjacent to solid blocks won't show the solid face.

**Impact:**
- Can't properly render glass, water, ice
- Visual holes when transparent blocks added
- Z-fighting with naive transparency

**Location:** `src/Content/ChunkMeshManager/ChunkMeshManager.cpp` - face culling logic

**Fix - Step by Step:**

1. **Add transparency property to blocks:**

```cpp
enum class BlockRenderType {
    OPAQUE,
    TRANSPARENT,
    CUTOUT  // Like leaves - either fully opaque or fully transparent
};

struct BlockProperties {
    Material material;
    BlockRenderType renderType;
    // ...
};
```

2. **Update face culling logic:**

```cpp
bool shouldRenderFace(Material current, Material neighbor) {
    auto currentType = BlockRegistry::getRenderType(current);
    auto neighborType = BlockRegistry::getRenderType(neighbor);

    if (neighbor == Material::AIR)
        return true;

    if (currentType == BlockRenderType::OPAQUE) {
        // Opaque blocks show faces against transparent neighbors
        return neighborType != BlockRenderType::OPAQUE;
    }

    if (currentType == BlockRenderType::TRANSPARENT) {
        // Transparent blocks show faces against different materials
        return current != neighbor;
    }

    return false;
}
```

3. **Separate opaque and transparent meshes:**

```cpp
struct ChunkMeshData {
    std::vector<Vertex> opaqueVertices;
    std::vector<Vertex> transparentVertices;
};

class ChunkMesh {
    VAO m_opaqueVAO;
    VAO m_transparentVAO;
    size_t m_opaqueCount;
    size_t m_transparentCount;
};
```

4. **Render in correct order:**

```cpp
void World::render() {
    // 1. Render all opaque geometry (depth write ON)
    glDepthMask(GL_TRUE);
    for (auto& mesh : m_meshes)
        mesh.renderOpaque();

    // 2. Render transparent geometry back-to-front (depth write OFF)
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Sort chunks by distance (far to near)
    std::sort(transparentChunks.begin(), transparentChunks.end(),
              [&](auto& a, auto& b) { return a.distance > b.distance; });

    for (auto& mesh : transparentChunks)
        mesh.renderTransparent();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
```

---

### 10. VAO/VBO Fragmentation

**Problem:**
Each chunk creates its own VAO and VBO. With hundreds of chunks, this leads to memory fragmentation and many small allocations.

**Impact:**
- GPU memory fragmentation
- Many small buffer objects
- Suboptimal memory access patterns

**Location:** `src/Content/ChunkMesh/ChunkMesh.h`

**Fix - Step by Step:**

1. **Create a chunk buffer pool:**

```cpp
class ChunkBufferPool {
    static constexpr size_t BLOCK_SIZE = 256 * 1024;  // 256KB blocks

    struct Block {
        GLuint buffer;
        size_t used;
        std::vector<bool> freeList;  // Track 4KB slots
    };

    std::vector<Block> m_blocks;

public:
    BufferAllocation allocate(size_t size);
    void free(BufferAllocation alloc);
};

struct BufferAllocation {
    GLuint buffer;
    size_t offset;
    size_t size;
};
```

2. **Allocate from pool:**

```cpp
BufferAllocation ChunkBufferPool::allocate(size_t size) {
    // Round up to 4KB alignment
    size = (size + 4095) & ~4095;

    // Find block with space
    for (auto& block : m_blocks) {
        if (block.used + size <= BLOCK_SIZE) {
            size_t offset = block.used;
            block.used += size;
            return { block.buffer, offset, size };
        }
    }

    // Create new block
    Block newBlock;
    glCreateBuffers(1, &newBlock.buffer);
    glNamedBufferStorage(newBlock.buffer, BLOCK_SIZE, nullptr,
                         GL_DYNAMIC_STORAGE_BIT);
    newBlock.used = size;
    m_blocks.push_back(newBlock);

    return { newBlock.buffer, 0, size };
}
```

3. **Use base vertex for drawing:**

```cpp
class ChunkMesh {
    BufferAllocation m_allocation;
    GLuint m_vao;  // Shared VAO with different buffer bindings

public:
    void render() {
        glBindVertexArray(m_vao);
        glBindVertexBuffer(0, m_allocation.buffer,
                          m_allocation.offset, sizeof(Vertex));
        glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
    }
};
```

4. **Defragmentation (optional):**

```cpp
void ChunkBufferPool::defragment() {
    // Periodically compact buffers
    // Move allocations to fill gaps
    // Requires updating ChunkMesh references
}
```

---

## Priority Matrix

| Issue | Impact | Effort | Priority |
|-------|--------|--------|----------|
| **Thread-Unsafe Blocks** | **CRITICAL** | Medium | **0 - FIX IMMEDIATELY** |
| Frustum Culling | High | Low | **1** |
| Greedy Meshing | High | Medium | **2** |
| Vertex Packing | Medium | Low | **3** |
| Dirty Flag | Medium | Low | **4** |
| Neighbor Dependency | Medium | Medium | **5** |
| Upload Throttling | Medium | Low | **6** |
| Transparent Blocks | Medium | Medium | **7** |
| Draw Call Batching | Medium | High | **8** |
| LOD System | Low | High | **9** |
| Buffer Pooling | Low | High | **10** |

---

## CRITICAL - Fix First

0. **Thread-unsafe block data** - Can cause crashes, data corruption, undefined behavior
   - Add `std::shared_mutex` to Chunk class
   - Use `std::shared_ptr<Chunk>` to prevent use-after-free
   - This is a **correctness bug**, not just performance

## Quick Wins (< 1 hour each)

1. **Frustum culling** - Immediate 40-70% draw call reduction
2. **Upload throttling** - Cap uploads per frame to prevent stuttering
3. **Dirty flag implementation** - Enable block modification support
4. **Vertex packing** - 80% memory reduction with shader changes

## Medium Effort (1-4 hours)

5. **Greedy meshing** - 10-50x vertex reduction
6. **Neighbor dependency tracking** - Fix chunk boundary artifacts
7. **Transparent block support** - Enable glass/water rendering

## Long Term (1+ days)

8. **Indirect draw calls** - Single draw call for entire world
9. **LOD system** - Extended draw distances
10. **Buffer pooling** - Reduced fragmentation

---

## Questions for Clarification

1. **Block modification**: Is runtime block editing planned? (affects dirty flag priority)
2. **Transparent blocks**: Will you need water/glass support? (affects render pipeline)
3. **Draw distance**: What's the target VIEW_DISTANCE? (affects LOD priority)
4. **Target hardware**: Minimum GPU specs? (affects optimization choices)
