# Re-Minecraft - TODO List

This document tracks pending optimizations and features for the chunk render pipeline.

---

## Completed Features

The following have been implemented:

- ~~**Frustum Culling**~~ - `src/Engine/Render/Frustum/Frustum.cpp`
- ~~**Shader Uniform Caching (MVP)**~~ - `src/Engine/Render/Shader/Shader.cpp`
- ~~**Face Culling Between Adjacent Chunks**~~ - `src/Content/ChunkMeshManager/ChunkMeshManager.cpp`
- ~~**Multi-Threaded Mesh Generation**~~ - `src/Content/ChunkMeshManager/ChunkMeshManager.cpp`
- ~~**Multi-Threaded Terrain Generation**~~ - `src/Content/ChunkManager/ChunkManager.cpp`
- ~~**Chunk State Management**~~ - `src/Content/Chunk/ChunkState.h`
- ~~**Priority Queue Chunk Prioritization**~~ - Distance-based priority in ThreadPool
- ~~**View Distance Limiting**~~ - `VIEW_DISTANCE = 8` in ChunkManager
- ~~**Texture Atlas (2D Array)**~~ - `src/Content/TextureRegistry/TextureRegistry.cpp`
- ~~**Double Buffering for Mesh Data**~~ - `src/Content/ChunkMesh/ChunkMesh.cpp`
- ~~**Atomic Block Buffer Swapping**~~ - `src/Content/Chunk/Chunk.cpp`
- ~~**GPU Back-Face Culling**~~ - `glCullFace(GL_BACK)` in Viewport
- ~~**Dirty Flag Chunk Remeshing**~~ - `isDirty()` checked in scheduleMeshing
- ~~**Chunk Neighbor Tracking**~~ - `ChunkNeighbors` struct and `getNeighbors()`
- ~~**Generation ID Cancellation**~~ - Stale job detection via generationID

---

## Pending Optimizations

### High Priority

#### 1. Greedy Meshing
**Impact:** 10-50x vertex reduction
**Effort:** Medium

Merge adjacent faces of the same material into larger quads instead of rendering each block face individually.

```
Current:           Greedy:
┌───┬───┬───┬───┐   ┌───────────────┐
│ 2▲│ 2▲│ 2▲│ 2▲│ → │      2▲       │
└───┴───┴───┴───┘   └───────────────┘
8 triangles         2 triangles
```

---

#### 2. Vertex Data Packing
**Impact:** 80% memory reduction
**Effort:** Low

Current vertex: 40 bytes. Pack into 8 bytes:
```cpp
struct PackedVertex {
    uint32_t positionAndNormal;  // x:5, y:5, z:5, normalIndex:3
    uint32_t uvAndTexture;       // u:5, v:5, texId:16
};
```

---

#### 3. Upload Queue Throttling
**Impact:** Eliminates frame stuttering
**Effort:** Low

Limit mesh uploads per frame to prevent lag spikes:
```cpp
constexpr int MAX_UPLOADS_PER_FRAME = 4;
```

---

### Medium Priority

#### 4. Transparent Block Handling
**Impact:** Enables glass/water rendering
**Effort:** Medium

- Separate opaque and transparent meshes
- Render transparent back-to-front with blending
- Update face culling to handle transparency

---

#### 5. Draw Call Batching / Indirect Rendering
**Impact:** Reduces CPU overhead
**Effort:** High

Use `glMultiDrawArraysIndirect()` to batch all chunks into a single draw call.

---

#### 6. Neighbor Dependency Remeshing
**Impact:** Fixes chunk boundary artifacts
**Effort:** Medium

Track which neighbors existed during mesh build. When a neighbor loads, trigger remesh for affected chunks.

---

### Low Priority

#### 7. Level of Detail (LOD)
**Impact:** Extended draw distances
**Effort:** High

Generate multiple mesh detail levels per chunk:
- FULL: distance < 32
- HALF: distance < 64
- QUARTER: distance < 128

---

#### 8. VAO/VBO Buffer Pooling
**Impact:** Reduced GPU memory fragmentation
**Effort:** High

Allocate chunks from a shared buffer pool instead of individual VBOs.

---

#### 9. GPU-Side Frustum Culling
**Impact:** Offloads CPU work
**Effort:** High

Use compute shaders to perform frustum tests on GPU.

---

## New Features

### Block Placement with Camera Facing Direction

**Description:** When placing blocks, rotate them based on the player's facing direction. This enables directional blocks (furnaces, chests, stairs, logs, etc.) to face the player.

**Implementation Steps:**

1. **Add rotation data to blocks:**
```cpp
enum class BlockRotation : uint8_t {
    NORTH = 0,  // -Z
    SOUTH = 1,  // +Z
    EAST  = 2,  // +X
    WEST  = 3,  // -X
    UP    = 4,  // +Y (for logs, pillars)
    DOWN  = 5   // -Y
};

struct BlockData {
    Material material;
    BlockRotation rotation;
};
```

2. **Calculate facing from camera yaw:**
```cpp
BlockRotation getFacingFromCamera(float yaw) {
    // Normalize yaw to 0-360
    yaw = fmod(fmod(yaw, 360.0f) + 360.0f, 360.0f);

    if (yaw >= 315.0f || yaw < 45.0f)   return BlockRotation::SOUTH;
    if (yaw >= 45.0f && yaw < 135.0f)   return BlockRotation::WEST;
    if (yaw >= 135.0f && yaw < 225.0f)  return BlockRotation::NORTH;
    return BlockRotation::EAST;
}
```

3. **Modify block placement in Player/World:**
```cpp
void Player::placeBlock() {
    auto hit = camera.raycast(world, 5.0f);
    if (!hit) return;

    glm::ivec3 placePos = hit->position + hit->normal;
    BlockRotation rotation = getFacingFromCamera(camera.getYaw());

    world.setBlock(placePos, selectedMaterial, rotation);
}
```

4. **Rotate UVs/vertices in mesh builder based on rotation:**
```cpp
void buildFaceMesh(..., BlockRotation rotation) {
    // Rotate texture coordinates or vertex positions
    // based on block rotation for directional textures
}
```

5. **Update vertex shader to handle rotation** (if using GPU rotation):
```glsl
// Pack rotation into vertex data (3 bits)
uint rotation = (packedData.x >> 18) & 0x7u;
mat3 rotMatrix = ROTATION_MATRICES[rotation];
normal = rotMatrix * normal;
```

---

## Priority Matrix

| Feature | Impact | Effort | Priority |
|---------|--------|--------|----------|
| Greedy Meshing | High | Medium | **1** |
| Vertex Packing | Medium | Low | **2** |
| Upload Throttling | Medium | Low | **3** |
| Transparent Blocks | Medium | Medium | **4** |
| Block Rotation | Medium | Medium | **5** |
| Draw Call Batching | Medium | High | **6** |
| Neighbor Remeshing | Medium | Medium | **7** |
| LOD System | Low | High | **8** |
| Buffer Pooling | Low | High | **9** |
| GPU Frustum Culling | Low | High | **10** |
