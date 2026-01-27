# Vertex Data Packing Optimization Guide

This document provides implementation steps and pseudo code for the vertex data packing optimization adapted to the re-minecraft architecture.

---

## Current State Analysis

### Current Vertex Structure (`src/Content/ChunkMesh/Vertex.h`)

```cpp
struct Vertex {
    glm::ivec3 position;  // 12 bytes (3 x int32)
    glm::ivec3 normal;    // 12 bytes (3 x int32)
    glm::ivec2 uv;        // 8 bytes  (2 x int32)
    unsigned int texId;   // 4 bytes  (1 x uint32)
};                        // Total: 36 bytes + padding = 40 bytes
```

### Problems with Current Approach

1. **Memory waste**: Position only needs 0-15 per axis (4 bits each), but uses 96 bits (3 x int32)
2. **Normal redundancy**: Only 6 possible normals, but uses 96 bits
3. **UV overkill**: Typically 0-1 range (or 0-15 with greedy meshing), uses 64 bits
4. **No rotation support**: No field for future block rotation feature

### Memory Impact (Example)

For a fully meshed chunk with ~5000 exposed faces (worst case):
- Current: `5000 faces x 6 vertices x 40 bytes = 1.2 MB per chunk`
- Packed: `5000 faces x 6 vertices x 8 bytes = 240 KB per chunk`
- **Savings: 80% reduction**

---

## Proposed Packed Format

### New Packed Vertex Structure

```cpp
struct PackedVertex {
    uint32_t data1;  // Position, Normal, Rotation, AO
    uint32_t data2;  // UV, Texture ID
};  // Total: 8 bytes
```

### Bit Layout

#### `data1` (32 bits) - Geometry Data

```
┌─────────────────────────────────────────────────────────────────┐
│ 31  30  29  28 │ 27  26  25  24 │ 23  22  21 │ 20  19  18 │ ...
│    AO (4 bits) │ reserved (4)   │ rotation   │   normal   │ ...
│     [0-15]     │    future      │   [0-5]    │   [0-5]    │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│ ... 17  16  15  14  13 │ 12  11  10   9   8 │  7   6   5   4   3 │  2   1   0 │
│        z (5 bits)      │     y (5 bits)     │     x (5 bits)     │ (continued)
│         [0-31]         │       [0-31]       │       [0-31]       │
└─────────────────────────────────────────────────────────────────┘
```

| Field    | Bits   | Range | Description                              |
|----------|--------|-------|------------------------------------------|
| x        | 0-4    | 0-31  | Local X position within chunk            |
| y        | 5-9    | 0-31  | Local Y position within chunk            |
| z        | 10-14  | 0-31  | Local Z position within chunk            |
| normal   | 15-17  | 0-5   | Normal direction index (see table below) |
| rotation | 18-20  | 0-5   | Block rotation (future feature)          |
| reserved | 21-27  | -     | Reserved for future use                  |
| ao       | 28-31  | 0-15  | Ambient occlusion level (future)         |

#### `data2` (32 bits) - Texture Data

```
┌─────────────────────────────────────────────────────────────────┐
│ 31 ... 26 │ 25 ... 10 │  9   8   7   6   5 │  4   3   2   1   0 │
│  reserved │  texId    │     v (5 bits)     │     u (5 bits)     │
│  (6 bits) │ (16 bits) │       [0-31]       │       [0-31]       │
└─────────────────────────────────────────────────────────────────┘
```

| Field    | Bits  | Range   | Description                           |
|----------|-------|---------|---------------------------------------|
| u        | 0-4   | 0-31    | U texture coordinate                  |
| v        | 5-9   | 0-31    | V texture coordinate                  |
| texId    | 10-25 | 0-65535 | Texture array layer index             |
| reserved | 26-31 | -       | Reserved (flags, animation frame, etc)|

### Normal Index Mapping

Matches your existing `MaterialFace` enum:

| Index | Direction | Normal Vector   |
|-------|-----------|-----------------|
| 0     | NORTH     | ( 0,  0, -1)    |
| 1     | SOUTH     | ( 0,  0, +1)    |
| 2     | WEST      | (-1,  0,  0)    |
| 3     | EAST      | (+1,  0,  0)    |
| 4     | UP        | ( 0, +1,  0)    |
| 5     | DOWN      | ( 0, -1,  0)    |

### Rotation Index Mapping (Future Feature)

| Index | Direction | Usage                          |
|-------|-----------|--------------------------------|
| 0     | NORTH     | Block faces -Z (default)       |
| 1     | SOUTH     | Block faces +Z                 |
| 2     | EAST      | Block faces +X                 |
| 3     | WEST      | Block faces -X                 |
| 4     | UP        | Block faces +Y (pillars/logs)  |
| 5     | DOWN      | Block faces -Y                 |

---

## Implementation Steps

### Step 1: Create the Packed Vertex Structure

Update `src/Content/ChunkMesh/Vertex.h`:

```cpp
#ifndef RE_MINECRAFT_VERTEX_H
#define RE_MINECRAFT_VERTEX_H

#include <glm/glm.hpp>
#include <cstdint>

// Legacy vertex (keep for GUI or other uses)
struct Vertex {
    glm::ivec3 position;
    glm::ivec3 normal;
    glm::ivec2 uv;
    unsigned int texId;
};

// Packed vertex for chunk meshes (8 bytes vs 40 bytes)
struct PackedVertex {
    uint32_t data1;  // position (15 bits) + normal (3 bits) + rotation (3 bits) + ao (4 bits)
    uint32_t data2;  // uv (10 bits) + texId (16 bits)

    PackedVertex() : data1(0), data2(0) {}

    PackedVertex(uint8_t x, uint8_t y, uint8_t z,
                 uint8_t normalIndex, uint8_t rotation,
                 uint8_t u, uint8_t v, uint16_t texId,
                 uint8_t ao = 0)
    {
        data1 = (x & 0x1F)
              | ((y & 0x1F) << 5)
              | ((z & 0x1F) << 10)
              | ((normalIndex & 0x7) << 15)
              | ((rotation & 0x7) << 18)
              | ((ao & 0xF) << 28);

        data2 = (u & 0x1F)
              | ((v & 0x1F) << 5)
              | ((static_cast<uint32_t>(texId) & 0xFFFF) << 10);
    }

    // Convenience factory for current code (no rotation)
    static PackedVertex create(uint8_t x, uint8_t y, uint8_t z,
                               uint8_t normalIndex,
                               uint8_t u, uint8_t v,
                               uint16_t texId)
    {
        return PackedVertex(x, y, z, normalIndex, 0, u, v, texId, 0);
    }
};

struct GuiVertex {
    glm::vec2 position;
    glm::vec4 color;
};

#endif //RE_MINECRAFT_VERTEX_H
```

### Step 2: Update Type Alias

Update `src/Content/ChunkMesh/ChunkMesh.h`:

```cpp
// Change this line:
using MeshData = std::vector<Vertex>;

// To:
using MeshData = std::vector<PackedVertex>;
```

### Step 3: Update VAO Attribute Setup

Update `src/Engine/Render/VAO/VAO.cpp`:

```cpp
void VAO::storeBlockData(const std::vector<PackedVertex>& data) const {
    this->vbo.addData(data);

    // Attribute 0: data1 (position + normal + rotation + ao)
    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(PackedVertex),
                           reinterpret_cast<void*>(offsetof(PackedVertex, data1)));

    // Attribute 1: data2 (uv + texId)
    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(PackedVertex),
                           reinterpret_cast<void*>(offsetof(PackedVertex, data2)));

    this->vbo.unbind();
}
```

Don't forget to update the header (`VAO.h`) signature as well:

```cpp
void storeBlockData(const std::vector<PackedVertex>& data) const;
```

### Step 4: Update the Vertex Shader

Update `resources/shaders/World/world.vert`:

```glsl
#version 460 core

// Packed input (2 x uint32 instead of 4 attributes)
layout (location = 0) in uint data1;
layout (location = 1) in uint data2;

uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 ModelMatrix;

out vec3 currentPos;
out vec3 currentNormal;
out vec2 currentUvs;
flat out uint currentTexIndex;
flat out uint currentRotation;  // For future use
flat out float currentAO;       // For future use

// Normal lookup table (matches MaterialFace enum)
const vec3 NORMALS[6] = vec3[6](
    vec3( 0.0,  0.0, -1.0),  // NORTH
    vec3( 0.0,  0.0,  1.0),  // SOUTH
    vec3(-1.0,  0.0,  0.0),  // WEST
    vec3( 1.0,  0.0,  0.0),  // EAST
    vec3( 0.0,  1.0,  0.0),  // UP
    vec3( 0.0, -1.0,  0.0)   // DOWN
);

void main()
{
    // Unpack data1: position (15 bits) + normal (3 bits) + rotation (3 bits) + ao (4 bits)
    uint x = data1 & 0x1Fu;
    uint y = (data1 >> 5) & 0x1Fu;
    uint z = (data1 >> 10) & 0x1Fu;
    uint normalIndex = (data1 >> 15) & 0x7u;
    uint rotation = (data1 >> 18) & 0x7u;
    uint ao = (data1 >> 28) & 0xFu;

    // Unpack data2: uv (10 bits) + texId (16 bits)
    uint u = data2 & 0x1Fu;
    uint v = (data2 >> 5) & 0x1Fu;
    uint texId = (data2 >> 10) & 0xFFFFu;

    // Build position
    vec3 localPos = vec3(float(x), float(y), float(z));
    vec4 worldPos = ModelMatrix * vec4(localPos, 1.0);

    // Output
    currentPos = worldPos.xyz;
    currentNormal = NORMALS[normalIndex];
    currentUvs = vec2(float(u), float(v));
    currentTexIndex = texId;
    currentRotation = rotation;  // Pass to fragment shader if needed
    currentAO = float(ao) / 15.0;  // Normalize AO to 0.0-1.0

    gl_Position = ProjectionMatrix * ViewMatrix * worldPos;
}
```

### Step 5: Update Mesh Generation Code

Find your mesh builder (likely in `ChunkMeshManager` or similar) and update the vertex creation.

**Before (conceptual):**
```cpp
void addFace(MeshData& mesh, const glm::ivec3& pos,
             MaterialFace face, unsigned int texId)
{
    // For each of the 6 vertices of 2 triangles...
    for (const auto& vertexOffset : FACE_VERTICES[face]) {
        Vertex v;
        v.position = pos + vertexOffset.position;
        v.normal = FACE_NORMALS[face];
        v.uv = vertexOffset.uv;
        v.texId = texId;
        mesh.push_back(v);
    }
}
```

**After:**
```cpp
void addFace(MeshData& mesh, const glm::ivec3& pos,
             MaterialFace face, uint16_t texId, uint8_t rotation = 0)
{
    // Face vertex data: position offset (0 or 1) and UV (0 or 1)
    // Define for each face type...
    static constexpr uint8_t FACE_VERTEX_DATA[6][6][5] = {
        // NORTH face (-Z) - 6 vertices, each: {dx, dy, dz, u, v}
        {{0,0,0, 0,0}, {1,1,0, 1,1}, {1,0,0, 1,0}, {0,0,0, 0,0}, {0,1,0, 0,1}, {1,1,0, 1,1}},
        // SOUTH face (+Z)
        {{0,0,1, 1,0}, {1,0,1, 0,0}, {1,1,1, 0,1}, {0,0,1, 1,0}, {1,1,1, 0,1}, {0,1,1, 1,1}},
        // WEST face (-X)
        {{0,0,0, 1,0}, {0,0,1, 0,0}, {0,1,1, 0,1}, {0,0,0, 1,0}, {0,1,1, 0,1}, {0,1,0, 1,1}},
        // EAST face (+X)
        {{1,0,0, 0,0}, {1,1,1, 1,1}, {1,0,1, 1,0}, {1,0,0, 0,0}, {1,1,0, 0,1}, {1,1,1, 1,1}},
        // UP face (+Y)
        {{0,1,0, 1,0}, {0,1,1, 1,1}, {1,1,1, 0,1}, {0,1,0, 1,0}, {1,1,1, 0,1}, {1,1,0, 0,0}},
        // DOWN face (-Y)
        {{0,0,0, 1,1}, {1,0,1, 0,0}, {0,0,1, 1,0}, {0,0,0, 1,1}, {1,0,0, 0,1}, {1,0,1, 0,0}},
    };

    const uint8_t x = static_cast<uint8_t>(pos.x);
    const uint8_t y = static_cast<uint8_t>(pos.y);
    const uint8_t z = static_cast<uint8_t>(pos.z);
    const uint8_t normalIndex = static_cast<uint8_t>(face);

    for (int i = 0; i < 6; ++i) {
        const auto& vd = FACE_VERTEX_DATA[face][i];
        mesh.emplace_back(
            x + vd[0], y + vd[1], z + vd[2],  // position
            normalIndex,                        // normal
            rotation,                           // rotation (0 for now)
            vd[3], vd[4],                       // uv
            texId                               // texture
        );
    }
}
```

### Step 6: Update Fragment Shader (Optional)

If you want to use AO or rotation in the fragment shader:

```glsl
#version 460 core

in vec3 currentPos;
in vec3 currentNormal;
in vec2 currentUvs;
flat in uint currentTexIndex;
flat in uint currentRotation;
flat in float currentAO;

out vec4 FragColor;

uniform sampler2DArray Textures;

void main()
{
    float ambient = 0.4;
    vec3 lightPos = vec3(64.0, 256.0, 64.0);

    vec3 normal = normalize(currentNormal);
    vec3 lightDirection = normalize(lightPos - currentPos);
    float diffuse = max(dot(normal, lightDirection), 0.0);

    vec4 texColor = texture(Textures, vec3(currentUvs, float(currentTexIndex)));

    // Apply ambient occlusion (future feature)
    float aoFactor = 1.0 - (1.0 - currentAO) * 0.3;  // Subtle AO

    FragColor = texColor * (diffuse + ambient) * aoFactor;
}
```

---

## Migration Checklist

1. [ ] Update `Vertex.h` - Add `PackedVertex` struct
2. [ ] Update `ChunkMesh.h` - Change `MeshData` typedef
3. [ ] Update `VAO.h` - Change function signature
4. [ ] Update `VAO.cpp` - Change attribute setup (2 attributes instead of 4)
5. [ ] Update `world.vert` - Unpack vertex data in shader
6. [ ] Update `world.frag` - Handle new outputs (optional, for AO)
7. [ ] Update mesh builder - Use `PackedVertex` constructor
8. [ ] Test with existing chunks

---

## Greedy Meshing Compatibility

The 5-bit UV fields (0-31 range) support greedy meshing where merged quads need UV coordinates larger than 1. For a 16-block chunk, UVs can range from 0-16, which fits comfortably.

If you need larger UV ranges (for very aggressive greedy meshing), you can:
1. Use the 6 reserved bits in `data2` to extend UV range
2. Or scale UVs in the shader: `actualUV = packedUV * uvScale`

---

## Future Rotation Implementation

When implementing block rotation, update the mesh builder to pass the rotation value:

```cpp
// In your block placement code
void World::setBlock(const glm::ivec3& pos, Material mat, BlockRotation rot) {
    // Store rotation in block data somehow (separate array or packed with material)
    // ...
}

// In mesh builder
void buildChunkMesh(Chunk& chunk, MeshData& mesh) {
    for (each visible face) {
        uint8_t rotation = chunk.getBlockRotation(x, y, z);
        addFace(mesh, pos, face, texId, rotation);
    }
}
```

The shader can then use the rotation value to:
1. Rotate texture coordinates for directional textures
2. Apply rotation matrices to normals for correct lighting

---

## Performance Comparison

| Metric              | Before (Vertex) | After (PackedVertex) | Improvement |
|---------------------|-----------------|----------------------|-------------|
| Bytes per vertex    | 40              | 8                    | 80% smaller |
| Vertices per MB     | 26,214          | 131,072              | 5x more     |
| GPU bandwidth       | High            | Low                  | 5x less     |
| Vertex attributes   | 4               | 2                    | 50% fewer   |
| Cache efficiency    | Poor            | Good                 | Significant |

---

## Troubleshooting

### Black screen after changes
- Verify attribute locations match between VAO setup and shader
- Check that `glVertexAttribIPointer` is used (not `glVertexAttribPointer`)
- Ensure stride is `sizeof(PackedVertex)` (8 bytes)

### Incorrect positions
- Verify bit masking operations in shader match packing code
- Check that position values are in 0-15 range (or 0-31 if using extra bit)

### Wrong textures
- Verify texture ID is being packed into bits 10-25 of data2
- Check shader unpacking uses correct bit shifts
