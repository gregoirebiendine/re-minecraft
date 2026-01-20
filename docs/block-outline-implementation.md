# Block Outline Implementation Guide

This document describes how to implement a wireframe outline around the block the player is looking at.

## Existing Infrastructure

The codebase already provides the necessary foundations:

- **Raycasting**: `Camera::raycast()` returns a `Hit` struct containing the looked-at block position
- **Shader system**: Supports MVP matrix uniforms via `Shader` class
- **Block interaction**: `Engine::handleInputs()` already uses raycast for break/place logic

## Implementation Steps

### 1. Create a Line Cube Mesh

Create a static wireframe cube mesh representing the 12 edges of a unit cube.

- Use `GL_LINES` draw mode (24 vertices total, 2 per edge)
- Vertices span from `(0,0,0)` to `(1,1,1)`
- Create once at startup, reuse every frame
- Store in a dedicated VAO/VBO (simpler vertex format: position only)

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
   - Draw wireframe cube with `glDrawArrays(GL_LINES, 0, 24)`
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

- **Line width**: Call `glLineWidth(2.0f)` for thicker lines (limited support on modern OpenGL/some drivers)
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
│   ├── Draw wireframe cube
│   └── Disable polygon offset
└── Swap buffers
```

## Performance Notes

- Adds one draw call per frame (24 vertices)
- Raycast already performed in `handleInputs()` - consider caching the result
- Static mesh, no per-frame vertex uploads needed
