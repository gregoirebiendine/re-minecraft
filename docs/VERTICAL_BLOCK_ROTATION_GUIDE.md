# Vertical Block Rotation Implementation Guide

This guide explains how to implement Minecraft-style vertical block rotation for blocks like oak logs, where the block's orientation depends on which face of an existing block you click.

## Current Architecture Summary

| Component | Location | Current State |
|-----------|----------|---------------|
| Raycast | `src/Engine/Raycast/Raycast.h` | No face detection |
| Block Placement | `src/Content/Player/Player.cpp` | Uses player direction only |
| Rotation Storage | `src/Content/BlockRegistry/Material.h` | 3 bits (0-7), only 0-3 used |
| Face Remapping | `Material.h::remapFaceForRotation()` | Horizontal only |
| Block Metadata | `src/Content/BlockRegistry/BlockRegistry.h` | `directional` boolean |

## Target Behavior (Minecraft Style)

When placing a log:
- **Click on UP/DOWN face** → Log is vertical (default, top faces up)
- **Click on NORTH/SOUTH face** → Log is horizontal, aligned on Z-axis
- **Click on EAST/WEST face** → Log is horizontal, aligned on X-axis

## Rotation Value Mapping

Use the 3-bit rotation field (0-7) to encode block axis orientation:

| Rotation | Axis | Description |
|----------|------|-------------|
| 0 | Y (default) | Log pointing UP/DOWN (vertical) |
| 1 | Z | Log pointing NORTH/SOUTH (horizontal) |
| 2 | X | Log pointing EAST/WEST (horizontal) |
| 3-7 | Reserved | Future use (e.g., upside-down variants) |

---

## Step 1: Add Hit Face to Raycast Structure

### File: `src/Engine/Raycast/Raycast.h`

Add a `hitFace` field to store which face of the block was hit:

```cpp
// Before
struct Hit {
    bool hit = false;
    glm::ivec3 pos;
    glm::ivec3 previousPos;
};

// After
struct Hit {
    bool hit = false;
    glm::ivec3 pos;
    glm::ivec3 previousPos;
    MaterialFace hitFace;  // Which face of the block was hit
};
```

**Note:** You'll need to include or forward-declare `MaterialFace` in this header.

---

## Step 2: Calculate Hit Face in Raycast

### File: `src/Engine/Camera/Camera.cpp`

Modify `Camera::raycast()` to determine which face was hit based on the difference between `pos` and `previousPos`:

```cpp
// Pseudo-code for calculating hit face
Raycast::Hit Camera::raycast(World& world) const
{
    // ... existing raycast logic ...

    if (world.getBlock(blockPos.x, blockPos.y, blockPos.z)) {
        // Calculate which face was hit
        glm::ivec3 diff = blockPos - previousBlockPos;
        MaterialFace hitFace = calculateHitFace(diff);

        return {
            true,
            blockPos,
            previousBlockPos,
            hitFace  // New field
        };
    }

    // ... rest of function ...
}

// Helper function to determine hit face from direction
MaterialFace calculateHitFace(const glm::ivec3& diff)
{
    // diff is the direction FROM previousPos TO hitPos
    // The face hit is the OPPOSITE of this direction

    if (diff.x == 1)  return WEST;   // Entered from -X, hit WEST face
    if (diff.x == -1) return EAST;   // Entered from +X, hit EAST face
    if (diff.y == 1)  return DOWN;   // Entered from -Y, hit DOWN face
    if (diff.y == -1) return UP;     // Entered from +Y, hit UP face
    if (diff.z == 1)  return NORTH;  // Entered from -Z, hit NORTH face
    if (diff.z == -1) return SOUTH;  // Entered from +Z, hit SOUTH face

    return UP;  // Default fallback
}
```

**Important:** The hit face is the face of the TARGET block that was clicked, which is the opposite direction of travel.

---

## Step 3: Add Rotation Type to BlockMeta

### File: `src/Content/BlockRegistry/BlockRegistry.h`

Add an enum and field to distinguish rotation behavior:

```cpp
// Add new enum
enum class RotationType : uint8_t {
    NONE,            // No rotation (dirt, stone, etc.)
    HORIZONTAL,      // Rotate on Y-axis only (furnace, chest)
    AXIS             // Rotate based on placement face (logs, pillars)
};

// Modify BlockMeta struct
struct BlockMeta {
    std::string registerNamespace;
    std::string blockName;
    bool transparent;
    bool directional;          // Keep for backward compatibility
    RotationType rotationType; // NEW: type of rotation behavior
    float hardness;
    BlockFaces blockFaces;

    // ... existing methods ...
};
```

---

## Step 4: Update Block Registration

### File: `src/Content/BlockRegistry/BlockRegistry.cpp`

Update oak_log (and similar blocks) registration:

```cpp
// Before
registerBlock({
    "core",
    "oak_log",
    false,
    false,  // directional
    1.5f,
    { /* faces */ }
});

// After
registerBlock({
    "core",
    "oak_log",
    false,
    true,                    // directional = true
    RotationType::AXIS,      // NEW: axis-based rotation
    1.5f,
    {
        {NORTH, "oak_log"},
        {SOUTH, "oak_log"},
        {WEST, "oak_log"},
        {EAST, "oak_log"},
        {UP, "oak_log_top"},
        {DOWN, "oak_log_top"}
    }
});
```

---

## Step 5: Add Axis Rotation Helper

### File: `src/Engine/Utils/DirectionUtils.h`

Add a function to convert hit face to axis rotation:

```cpp
// Convert the face that was clicked to a block axis rotation
// For axis-aligned blocks like logs
inline BlockRotation getAxisFromHitFace(const MaterialFace hitFace)
{
    switch (hitFace)
    {
        case UP:
        case DOWN:
            return 0;  // Y-axis (vertical log)
        case NORTH:
        case SOUTH:
            return 1;  // Z-axis (horizontal, N/S aligned)
        case EAST:
        case WEST:
            return 2;  // X-axis (horizontal, E/W aligned)
        default:
            return 0;
    }
}
```

---

## Step 6: Update Block Placement Logic

### File: `src/Content/Player/Player.cpp`

Modify `placeBlock()` to handle different rotation types:

```cpp
void Player::placeBlock(World& world) const
{
    const auto& meta = this->blockRegistry.get(this->selectedBlockId);
    BlockRotation rotation = 0;

    switch (meta.rotationType)
    {
        case RotationType::NONE:
            // No rotation needed
            rotation = 0;
            break;

        case RotationType::HORIZONTAL:
            // Existing behavior: face opposite to player
            {
                const auto playerFacing = DirectionUtils::getHorizontalFacing(
                    this->camera.getForwardVector()
                );
                rotation = DirectionUtils::getOppositeFacing(playerFacing);
            }
            break;

        case RotationType::AXIS:
            // NEW: rotation based on which face was clicked
            rotation = DirectionUtils::getAxisFromHitFace(this->lastRaycast.hitFace);
            break;
    }

    const auto packedMaterial = BlockData::packBlockData(this->selectedBlockId, rotation);

    world.setBlock(
        this->lastRaycast.previousPos.x,
        this->lastRaycast.previousPos.y,
        this->lastRaycast.previousPos.z,
        packedMaterial
    );
}
```

---

## Step 7: Add Axis-Based Face Remapping

### File: `src/Content/BlockRegistry/Material.h`

Add a new remapping function for axis-rotated blocks:

```cpp
// Remap faces for axis-rotated blocks (logs, pillars)
// rotation: 0 = Y-axis, 1 = Z-axis, 2 = X-axis
inline MaterialFace remapFaceForAxisRotation(const MaterialFace face, const BlockRotation rotation)
{
    // Rotation 0: Y-axis (default) - no remapping needed
    if (rotation == 0)
        return face;

    // Rotation 1: Z-axis (log pointing N/S)
    // UP/DOWN become NORTH/SOUTH, NORTH/SOUTH become UP/DOWN
    if (rotation == 1)
    {
        switch (face)
        {
            case UP:    return SOUTH;
            case DOWN:  return NORTH;
            case NORTH: return DOWN;
            case SOUTH: return UP;
            default:    return face;  // EAST/WEST unchanged
        }
    }

    // Rotation 2: X-axis (log pointing E/W)
    // UP/DOWN become EAST/WEST, EAST/WEST become UP/DOWN
    if (rotation == 2)
    {
        switch (face)
        {
            case UP:    return EAST;
            case DOWN:  return WEST;
            case EAST:  return DOWN;
            case WEST:  return UP;
            default:    return face;  // NORTH/SOUTH unchanged
        }
    }

    return face;
}
```

---

## Step 8: Update Mesh Builder

### File: `src/Content/ChunkMeshManager/ChunkMeshManager.cpp`

Modify `getTextureFromRotation()` to handle axis rotation:

```cpp
std::string ChunkMeshManager::getTextureFromRotation(
    const BlockMeta& meta,
    const MaterialFace face,
    const BlockRotation rotation
)
{
    if (!meta.directional)
        return meta.getFaceTexture(face);

    // Choose remapping based on rotation type
    if (meta.rotationType == RotationType::AXIS)
    {
        return meta.getFaceTexture(
            BlockData::remapFaceForAxisRotation(face, rotation)
        );
    }

    // Default: horizontal rotation
    return meta.getFaceTexture(
        BlockData::remapFaceForRotation(face, rotation)
    );
}
```

---

## Step 9: Update Fragment Shader (Optional)

### File: `resources/shaders/World/world.frag`

The current shader only rotates UVs for UP/DOWN faces. For axis-rotated blocks, the face remapping handles texture assignment, so **no shader changes are required** for basic log behavior.

However, if you want the top texture of horizontal logs to rotate based on player direction (like Minecraft), you would need additional logic:

```glsl
// Optional: Only apply UV rotation for specific cases
void main()
{
    vec2 texCoords = currentUvs;

    // Only rotate UVs for vertical faces of vertically-oriented blocks
    // For axis-rotated blocks, face remapping handles texture selection
    if (currentNormal.y != 0.0 && currentRotation < 4u)
        texCoords = rotateUV(currentUvs, currentRotation);

    // ... rest of shader ...
}
```

---

## Summary of Changes

| Step | File | Change |
|------|------|--------|
| 1 | `Raycast.h` | Add `hitFace` to `Hit` struct |
| 2 | `Camera.cpp` | Calculate hit face in raycast |
| 3 | `BlockRegistry.h` | Add `RotationType` enum and field |
| 4 | `BlockRegistry.cpp` | Update log registration |
| 5 | `DirectionUtils.h` | Add `getAxisFromHitFace()` |
| 6 | `Player.cpp` | Handle `RotationType::AXIS` in placement |
| 7 | `Material.h` | Add `remapFaceForAxisRotation()` |
| 8 | `ChunkMeshManager.cpp` | Use axis remapping for logs |
| 9 | `world.frag` | (Optional) Adjust UV rotation logic |

---

## Testing Checklist

- [ ] Place log on ground (click UP face) → Log is vertical
- [ ] Place log on ceiling (click DOWN face) → Log is vertical
- [ ] Place log on north wall (click SOUTH face) → Log is horizontal (Z-axis)
- [ ] Place log on east wall (click WEST face) → Log is horizontal (X-axis)
- [ ] Verify log textures show correctly (top texture on ends, bark on sides)
- [ ] Verify furnace still rotates horizontally based on player direction
- [ ] Verify non-directional blocks (stone, dirt) are unaffected

---

## Visual Reference

```
Rotation 0 (Y-axis):     Rotation 1 (Z-axis):     Rotation 2 (X-axis):
      [TOP]                   [BARK]                   [BARK]
       ___                     ___                      ___
      /   \                   /   \                    /   \
     |BARK |                 |TOP  |                  | TOP|
     |     |                 |     |                  |    |
      \___/                   \___/                    \___/
      [TOP]                   [BARK]                   [BARK]
    (vertical)            (pointing N/S)           (pointing E/W)
```
