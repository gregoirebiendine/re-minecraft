# Block Outline Implementation Guide

This document describes how to implement a wireframe outline around the block the player is looking at.

## Existing Infrastructure

The codebase already provides the necessary foundations:

- **Raycasting**: `Camera::raycast()` returns a `Hit` struct containing the looked-at block position
- **Shader system**: Supports MVP matrix uniforms via `Shader` class
- **Block interaction**: `Engine::handleInputs()` already uses raycast for break/place logic

## Implementation Steps

### 1. Create a Line Cube Mesh

Create a static wireframe cube mesh representing the 12 edges of a unit cube using 3D beams for configurable line width.

- Use `GL_TRIANGLES` draw mode (288 vertices total: 12 edges × 4 faces × 2 triangles × 3 vertices)
- Vertices span from `(-w, -w, -w)` to `(1+w, 1+w, 1+w)` where `w` is half the line width
- Create once at startup, reuse every frame
- Store in a dedicated VAO/VBO (position only)

**Vertex Generation (with configurable width `w`):**

Each edge becomes a rectangular beam with 4 faces (no end caps), ensuring consistent appearance from all viewing angles:
- X-axis edges: 4 faces expanding in Y and Z
- Y-axis edges: 4 faces expanding in X and Z
- Z-axis edges: 4 faces expanding in X and Y

```cpp
const float W = 0.005f; // half line width

float vertices[] = {
    // ============================================================
    // BOTTOM FACE EDGES (4 edges)
    // ============================================================

    // Edge: (0,0,0) -> (1,0,0) along X
    // Face -Z
    -W, -W, -W,   1+W, -W, -W,   1+W, +W, -W,
    -W, -W, -W,   1+W, +W, -W,   -W, +W, -W,
    // Face +Z
    -W, -W, +W,   -W, +W, +W,   1+W, +W, +W,
    -W, -W, +W,   1+W, +W, +W,   1+W, -W, +W,
    // Face -Y
    -W, -W, -W,   -W, -W, +W,   1+W, -W, +W,
    -W, -W, -W,   1+W, -W, +W,   1+W, -W, -W,
    // Face +Y
    -W, +W, -W,   1+W, +W, -W,   1+W, +W, +W,
    -W, +W, -W,   1+W, +W, +W,   -W, +W, +W,

    // Edge: (1,0,0) -> (1,1,0) along Y
    // Face -Z
    1-W, -W, -W,   1+W, -W, -W,   1+W, 1+W, -W,
    1-W, -W, -W,   1+W, 1+W, -W,   1-W, 1+W, -W,
    // Face +Z
    1-W, -W, +W,   1-W, 1+W, +W,   1+W, 1+W, +W,
    1-W, -W, +W,   1+W, 1+W, +W,   1+W, -W, +W,
    // Face -X
    1-W, -W, -W,   1-W, 1+W, -W,   1-W, 1+W, +W,
    1-W, -W, -W,   1-W, 1+W, +W,   1-W, -W, +W,
    // Face +X
    1+W, -W, -W,   1+W, -W, +W,   1+W, 1+W, +W,
    1+W, -W, -W,   1+W, 1+W, +W,   1+W, 1+W, -W,

    // Edge: (1,1,0) -> (0,1,0) along X
    // Face -Z
    -W, 1-W, -W,   1+W, 1-W, -W,   1+W, 1+W, -W,
    -W, 1-W, -W,   1+W, 1+W, -W,   -W, 1+W, -W,
    // Face +Z
    -W, 1-W, +W,   -W, 1+W, +W,   1+W, 1+W, +W,
    -W, 1-W, +W,   1+W, 1+W, +W,   1+W, 1-W, +W,
    // Face -Y
    -W, 1-W, -W,   -W, 1-W, +W,   1+W, 1-W, +W,
    -W, 1-W, -W,   1+W, 1-W, +W,   1+W, 1-W, -W,
    // Face +Y
    -W, 1+W, -W,   1+W, 1+W, -W,   1+W, 1+W, +W,
    -W, 1+W, -W,   1+W, 1+W, +W,   -W, 1+W, +W,

    // Edge: (0,1,0) -> (0,0,0) along Y
    // Face -Z
    -W, -W, -W,   +W, -W, -W,   +W, 1+W, -W,
    -W, -W, -W,   +W, 1+W, -W,   -W, 1+W, -W,
    // Face +Z
    -W, -W, +W,   -W, 1+W, +W,   +W, 1+W, +W,
    -W, -W, +W,   +W, 1+W, +W,   +W, -W, +W,
    // Face -X
    -W, -W, -W,   -W, 1+W, -W,   -W, 1+W, +W,
    -W, -W, -W,   -W, 1+W, +W,   -W, -W, +W,
    // Face +X
    +W, -W, -W,   +W, -W, +W,   +W, 1+W, +W,
    +W, -W, -W,   +W, 1+W, +W,   +W, 1+W, -W,

    // ============================================================
    // TOP FACE EDGES (4 edges)
    // ============================================================

    // Edge: (0,0,1) -> (1,0,1) along X
    // Face -Z
    -W, -W, 1-W,   1+W, -W, 1-W,   1+W, +W, 1-W,
    -W, -W, 1-W,   1+W, +W, 1-W,   -W, +W, 1-W,
    // Face +Z
    -W, -W, 1+W,   -W, +W, 1+W,   1+W, +W, 1+W,
    -W, -W, 1+W,   1+W, +W, 1+W,   1+W, -W, 1+W,
    // Face -Y
    -W, -W, 1-W,   -W, -W, 1+W,   1+W, -W, 1+W,
    -W, -W, 1-W,   1+W, -W, 1+W,   1+W, -W, 1-W,
    // Face +Y
    -W, +W, 1-W,   1+W, +W, 1-W,   1+W, +W, 1+W,
    -W, +W, 1-W,   1+W, +W, 1+W,   -W, +W, 1+W,

    // Edge: (1,0,1) -> (1,1,1) along Y
    // Face -Z
    1-W, -W, 1-W,   1+W, -W, 1-W,   1+W, 1+W, 1-W,
    1-W, -W, 1-W,   1+W, 1+W, 1-W,   1-W, 1+W, 1-W,
    // Face +Z
    1-W, -W, 1+W,   1-W, 1+W, 1+W,   1+W, 1+W, 1+W,
    1-W, -W, 1+W,   1+W, 1+W, 1+W,   1+W, -W, 1+W,
    // Face -X
    1-W, -W, 1-W,   1-W, 1+W, 1-W,   1-W, 1+W, 1+W,
    1-W, -W, 1-W,   1-W, 1+W, 1+W,   1-W, -W, 1+W,
    // Face +X
    1+W, -W, 1-W,   1+W, -W, 1+W,   1+W, 1+W, 1+W,
    1+W, -W, 1-W,   1+W, 1+W, 1+W,   1+W, 1+W, 1-W,

    // Edge: (1,1,1) -> (0,1,1) along X
    // Face -Z
    -W, 1-W, 1-W,   1+W, 1-W, 1-W,   1+W, 1+W, 1-W,
    -W, 1-W, 1-W,   1+W, 1+W, 1-W,   -W, 1+W, 1-W,
    // Face +Z
    -W, 1-W, 1+W,   -W, 1+W, 1+W,   1+W, 1+W, 1+W,
    -W, 1-W, 1+W,   1+W, 1+W, 1+W,   1+W, 1-W, 1+W,
    // Face -Y
    -W, 1-W, 1-W,   -W, 1-W, 1+W,   1+W, 1-W, 1+W,
    -W, 1-W, 1-W,   1+W, 1-W, 1+W,   1+W, 1-W, 1-W,
    // Face +Y
    -W, 1+W, 1-W,   1+W, 1+W, 1-W,   1+W, 1+W, 1+W,
    -W, 1+W, 1-W,   1+W, 1+W, 1+W,   -W, 1+W, 1+W,

    // Edge: (0,1,1) -> (0,0,1) along Y
    // Face -Z
    -W, -W, 1-W,   +W, -W, 1-W,   +W, 1+W, 1-W,
    -W, -W, 1-W,   +W, 1+W, 1-W,   -W, 1+W, 1-W,
    // Face +Z
    -W, -W, 1+W,   -W, 1+W, 1+W,   +W, 1+W, 1+W,
    -W, -W, 1+W,   +W, 1+W, 1+W,   +W, -W, 1+W,
    // Face -X
    -W, -W, 1-W,   -W, 1+W, 1-W,   -W, 1+W, 1+W,
    -W, -W, 1-W,   -W, 1+W, 1+W,   -W, -W, 1+W,
    // Face +X
    +W, -W, 1-W,   +W, -W, 1+W,   +W, 1+W, 1+W,
    +W, -W, 1-W,   +W, 1+W, 1+W,   +W, 1+W, 1-W,

    // ============================================================
    // VERTICAL EDGES (4 edges along Z)
    // ============================================================

    // Edge: (0,0,0) -> (0,0,1) along Z
    // Face -X
    -W, -W, -W,   -W, +W, -W,   -W, +W, 1+W,
    -W, -W, -W,   -W, +W, 1+W,   -W, -W, 1+W,
    // Face +X
    +W, -W, -W,   +W, -W, 1+W,   +W, +W, 1+W,
    +W, -W, -W,   +W, +W, 1+W,   +W, +W, -W,
    // Face -Y
    -W, -W, -W,   -W, -W, 1+W,   +W, -W, 1+W,
    -W, -W, -W,   +W, -W, 1+W,   +W, -W, -W,
    // Face +Y
    -W, +W, -W,   +W, +W, -W,   +W, +W, 1+W,
    -W, +W, -W,   +W, +W, 1+W,   -W, +W, 1+W,

    // Edge: (1,0,0) -> (1,0,1) along Z
    // Face -X
    1-W, -W, -W,   1-W, +W, -W,   1-W, +W, 1+W,
    1-W, -W, -W,   1-W, +W, 1+W,   1-W, -W, 1+W,
    // Face +X
    1+W, -W, -W,   1+W, -W, 1+W,   1+W, +W, 1+W,
    1+W, -W, -W,   1+W, +W, 1+W,   1+W, +W, -W,
    // Face -Y
    1-W, -W, -W,   1-W, -W, 1+W,   1+W, -W, 1+W,
    1-W, -W, -W,   1+W, -W, 1+W,   1+W, -W, -W,
    // Face +Y
    1-W, +W, -W,   1+W, +W, -W,   1+W, +W, 1+W,
    1-W, +W, -W,   1+W, +W, 1+W,   1-W, +W, 1+W,

    // Edge: (1,1,0) -> (1,1,1) along Z
    // Face -X
    1-W, 1-W, -W,   1-W, 1+W, -W,   1-W, 1+W, 1+W,
    1-W, 1-W, -W,   1-W, 1+W, 1+W,   1-W, 1-W, 1+W,
    // Face +X
    1+W, 1-W, -W,   1+W, 1-W, 1+W,   1+W, 1+W, 1+W,
    1+W, 1-W, -W,   1+W, 1+W, 1+W,   1+W, 1+W, -W,
    // Face -Y
    1-W, 1-W, -W,   1-W, 1-W, 1+W,   1+W, 1-W, 1+W,
    1-W, 1-W, -W,   1+W, 1-W, 1+W,   1+W, 1-W, -W,
    // Face +Y
    1-W, 1+W, -W,   1+W, 1+W, -W,   1+W, 1+W, 1+W,
    1-W, 1+W, -W,   1+W, 1+W, 1+W,   1-W, 1+W, 1+W,

    // Edge: (0,1,0) -> (0,1,1) along Z
    // Face -X
    -W, 1-W, -W,   -W, 1+W, -W,   -W, 1+W, 1+W,
    -W, 1-W, -W,   -W, 1+W, 1+W,   -W, 1-W, 1+W,
    // Face +X
    +W, 1-W, -W,   +W, 1-W, 1+W,   +W, 1+W, 1+W,
    +W, 1-W, -W,   +W, 1+W, 1+W,   +W, 1+W, -W,
    // Face -Y
    -W, 1-W, -W,   -W, 1-W, 1+W,   +W, 1-W, 1+W,
    -W, 1-W, -W,   +W, 1-W, 1+W,   +W, 1-W, -W,
    // Face +Y
    -W, 1+W, -W,   +W, 1+W, -W,   +W, 1+W, 1+W,
    -W, 1+W, -W,   +W, 1+W, 1+W,   -W, 1+W, 1+W,
};
```

### 2. Create an Outline Shader

A minimal shader pair separate from the world shader:

**Vertex Shader**
- Input: `vec3 position`
- Uniforms: `mat4 model`, `mat4 view`, `mat4 projection`
- Transform position by MVP matrix

**Fragment Shader**
- Output a solid color (black, white, or a contrasting color like cyan)
- Optionally add slight transparency for a softer look

### 3. Integrate into Render Pipeline

In `Engine::render()`, after drawing the world:

1. Perform raycast from camera position
2. If `hit.hit == true`:
   - Calculate model matrix as translation to `hit.pos`
   - Bind outline shader
   - Set MVP uniforms
   - Apply depth offset to prevent z-fighting
   - Draw outline cube with `glDrawArrays(GL_TRIANGLES, 0, 288)`
   - Reset depth offset

### 4. Handle Depth Fighting

The outline and block faces occupy the same depth, causing z-fighting. Solutions:

| Method | Description |
|--------|-------------|
| `glPolygonOffset(-1, -1)` | Push lines slightly toward camera |
| Scale outline | Use scale 1.002 instead of 1.0 |
| Disable depth test | Draw outline without depth test (may look odd) |

Recommended: Use `glPolygonOffset` or slight scale increase.

### 5. Optional Enhancements

- **Line width**: Adjust the `W` constant when generating vertices (rebuilds mesh)
- **Dynamic color**: Choose outline color based on block brightness
- **Animation**: Subtle pulsing or breathing effect on the outline

## File Organization

| Component | Suggested Location |
|-----------|-------------------|
| Outline mesh class | `src/Engine/Render/OutlineMesh.h` |
| Outline shader | `res/shaders/outline.vert`, `outline.frag` |
| Hit state storage | `Engine` class member (`Hit m_lastHit`) |
| Render call | `Engine::render()` after world rendering |

## Render Order

```
Engine::render()
├── Clear buffers
├── Bind world shader
├── Render chunks (opaque geometry)
├── Render block outline    <-- Add here
│   ├── Raycast from camera
│   ├── If hit, bind outline shader
│   ├── Set model matrix to hit.pos
│   ├── Enable polygon offset
│   ├── Draw outline cube
│   └── Disable polygon offset
└── Swap buffers
```

## Performance Notes

- Adds one draw call per frame (288 vertices)
- Raycast already performed in `handleInputs()` - consider caching the result
- Static mesh, no per-frame vertex uploads needed
