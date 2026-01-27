# Block Placement with Camera Facing Direction

This guide explains how to implement directional block placement where blocks like furnaces, crafting tables, or chests face the player when placed.

---

## Overview

**Goal**: When a player places certain blocks, the block should rotate to face the player (opposite of camera direction).

**Current State**:
- `Material` (block ID) is stored as `unsigned short` (16 bits)
- `PackedBlockVertex` already has rotation bits (18-20) reserved but unused
- `buildFaceMesh()` accepts a rotation parameter but always receives 0
- Chunks store only block IDs, no rotation data

---

## Architecture Decision: Storing Block Rotation

You need to store rotation per-block. There are three approaches:

### Option A: Pack Rotation into Material (Recommended)

Use 13 bits for block ID + 3 bits for rotation within the existing `unsigned short`.

```
Material (16 bits):
┌──────────────────────────────────────────┐
│ 15 14 13 │ 12 11 10 9 8 7 6 5 4 3 2 1 0 │
│ rotation │         block ID             │
│  (3 bits)│         (13 bits)            │
│   0-5    │         0-8191               │
└──────────────────────────────────────────┘
```

**Pros**: No memory overhead, backwards compatible, simple
**Cons**: Limits block types to 8191 (still plenty)

### Option B: Separate Rotation Array

Add a parallel `std::array<uint8_t, VOLUME>` in Chunk for rotations.

**Pros**: Full 65535 block types, cleaner separation
**Cons**: +4KB per chunk, more data to sync/copy

### Option C: Sparse Storage (Map)

Only store rotation for blocks that have it (most blocks are rotation-less).

**Pros**: Memory efficient for worlds with few directional blocks
**Cons**: Map lookups slower, more complex mesh generation

**Recommendation**: Use **Option A** (pack into Material) for simplicity and zero memory overhead.

---

## Step 1: Create BlockData Helpers

Create utility functions to pack/unpack rotation with block ID.

**Pseudo code** (`BlockData.h` or add to `Material.h`):

```
// Constants
ROTATION_BITS     = 3
ROTATION_SHIFT    = 13
BLOCK_ID_MASK     = 0x1FFF  (bits 0-12)
ROTATION_MASK     = 0x7     (bits 0-2 of rotation)

// Pack block ID + rotation into Material
function packBlockData(blockId, rotation):
    return (rotation << ROTATION_SHIFT) | (blockId & BLOCK_ID_MASK)

// Extract block ID from Material
function getBlockId(material):
    return material & BLOCK_ID_MASK

// Extract rotation from Material
function getRotation(material):
    return (material >> ROTATION_SHIFT) & ROTATION_MASK
```

**Rotation values** (horizontal only for now):
| Value | Direction | Player Faces | Block Faces |
|-------|-----------|--------------|-------------|
| 0     | NORTH     | -Z           | +Z (toward player) |
| 1     | SOUTH     | +Z           | -Z |
| 2     | WEST      | -X           | +X |
| 3     | EAST      | +X           | -X |

---

## Step 2: Calculate Facing Direction from Camera

When the player places a block, determine which direction they're facing.

**Pseudo code** (add to `Player.cpp` or create `DirectionUtils`):

```
// Convert camera yaw to horizontal facing direction (4 directions)
function getHorizontalFacing(yaw):
    // Normalize yaw to 0-360
    normalizedYaw = yaw mod 360
    if normalizedYaw < 0:
        normalizedYaw += 360

    // Divide into 4 quadrants (each 90 degrees)
    // Offset by 45 degrees so boundaries are at diagonals
    if normalizedYaw >= 315 OR normalizedYaw < 45:
        return SOUTH   // Camera looking +Z, block faces -Z
    else if normalizedYaw >= 45 AND normalizedYaw < 135:
        return WEST    // Camera looking +X, block faces -X
    else if normalizedYaw >= 135 AND normalizedYaw < 225:
        return NORTH   // Camera looking -Z, block faces +Z
    else:
        return EAST    // Camera looking -X, block faces +X

// Get the opposite direction (block faces player)
function getOppositeFacing(facing):
    switch facing:
        NORTH -> SOUTH
        SOUTH -> NORTH
        EAST  -> WEST
        WEST  -> EAST
```

**Note**: The block should face the player, so use the **opposite** of camera direction.

---

## Step 3: Mark Blocks as Directional

Not all blocks should rotate. Add a flag to `BlockMeta` in `BlockRegistry`.

**Pseudo code** (modify `BlockMeta` struct):

```
struct BlockMeta:
    // existing fields...
    registerNamespace
    blockName
    transparent
    hardness
    blockFaces

    // NEW: whether this block rotates based on placement
    isDirectional = false
```

When registering blocks like furnace, chest, etc.:

```
blockRegistry.register("furnace", {
    // ... other properties
    isDirectional: true
})
```

---

## Step 4: Modify Block Placement

Update `Player::handleInputs()` to include rotation when placing directional blocks.

**Current flow**:
```
player places block
  -> get raycast hit position
  -> world.setBlock(position, selectedMaterial)
```

**New flow**:
```
player places block
  -> get raycast hit position
  -> get block metadata for selectedMaterial
  -> if block.isDirectional:
       playerFacing = getHorizontalFacing(camera.yaw)
       blockFacing = getOppositeFacing(playerFacing)
       rotation = facingToRotation(blockFacing)
       packedMaterial = packBlockData(selectedMaterial, rotation)
     else:
       packedMaterial = packBlockData(selectedMaterial, 0)
  -> world.setBlock(position, packedMaterial)
```

**Pseudo code** (modify placement in `Player.cpp`):

```
function handleBlockPlacement():
    hit = camera.raycast(world)
    if not hit.valid:
        return

    blockId = selectedMaterial
    rotation = 0

    meta = blockRegistry.get(blockId)
    if meta.isDirectional:
        playerFacing = getHorizontalFacing(camera.getYaw())
        blockFacing = getOppositeFacing(playerFacing)
        rotation = blockFacing  // 0=NORTH, 1=SOUTH, 2=WEST, 3=EAST

    packedMaterial = packBlockData(blockId, rotation)
    world.setBlock(hit.previousPos, packedMaterial)
```

---

## Step 5: Update Mesh Generation

Modify `ChunkMeshManager::buildMeshJob()` to extract and use rotation.

**Current code** (simplified):

```
for each block in chunk:
    material = blockData[index]
    if material == 0: continue  // air

    meta = blockRegistry.get(material)
    for each visible face:
        buildFaceMesh(mesh, pos, face, textureId, 0)  // rotation always 0
```

**New code**:

```
for each block in chunk:
    material = blockData[index]
    if material == 0: continue

    blockId = getBlockId(material)
    rotation = getRotation(material)

    meta = blockRegistry.get(blockId)

    for each visible face:
        // Map the requested face through rotation
        if meta.isDirectional:
            mappedFace = remapFaceForRotation(face, rotation)
            texture = meta.getFaceTexture(mappedFace)
        else:
            texture = meta.getFaceTexture(face)

        buildFaceMesh(mesh, pos, face, textureId, rotation)
```

---

## Step 6: Face Remapping Logic

When a block is rotated, its textures need to be remapped so the "front" texture appears on the correct world-space face.

**Concept**:
- A furnace has a "front" texture (with fire) defined for its NORTH face in the registry
- When placed facing SOUTH, the front texture should appear on the SOUTH world face
- We remap: "which registry face should we use for this world face?"

**Pseudo code**:

```
// Remap face based on block rotation
// rotation: 0=NORTH, 1=SOUTH, 2=WEST, 3=EAST (the direction block faces)
function remapFaceForRotation(worldFace, rotation):
    // UP and DOWN never change
    if worldFace == UP or worldFace == DOWN:
        return worldFace

    // For horizontal faces, we need a rotation table
    // This maps: given the block's facing direction and the world face,
    // which "local" face texture should we use?

    // Rotation table [rotation][worldFace] -> localFace
    // rotation 0 = block faces NORTH (default, no rotation)
    // rotation 1 = block faces SOUTH (180 degrees)
    // rotation 2 = block faces WEST (90 degrees CCW)
    // rotation 3 = block faces EAST (90 degrees CW)

    FACE_REMAP = [
        // rot=0 (facing NORTH): no change
        [NORTH, SOUTH, WEST, EAST],

        // rot=1 (facing SOUTH): 180 degree rotation
        [SOUTH, NORTH, EAST, WEST],

        // rot=2 (facing WEST): 90 CCW
        [EAST, WEST, NORTH, SOUTH],

        // rot=3 (facing EAST): 90 CW
        [WEST, EAST, SOUTH, NORTH]
    ]

    return FACE_REMAP[rotation][worldFace]
```

**Example**: Furnace with front on NORTH face in registry:
- Placed facing SOUTH (rotation=1)
- World's SOUTH face should show the front
- `remapFaceForRotation(SOUTH, 1)` returns `NORTH`
- We fetch texture from registry's NORTH face (the front)

---

## Step 7: Update Shader for UV Rotation

The rotation value is passed from vertex to fragment shader. For blocks like planks, the top/bottom face textures should rotate to align with the player's facing direction.

**Implementation** (in `world.frag`):

```glsl
flat in uint currentRotation;

// Rotate UV coordinates by 90 degree increments
vec2 rotateUV(vec2 uv, uint rotation) {
    vec2 centered = uv - 0.5;
    float angle = float(rotation) * 1.5708; // 90 degrees in radians
    float c = cos(angle);
    float s = sin(angle);
    vec2 rotated = vec2(
        centered.x * c - centered.y * s,
        centered.x * s + centered.y * c
    );
    return rotated + 0.5;
}

// Apply to top/bottom faces only
vec2 texCoords = currentUvs;
if (currentNormal.y != 0.0) {
    texCoords = rotateUV(currentUvs, currentRotation);
}
```

**Note**: Don't forget to pass `currentRotation` from the vertex shader:
```glsl
flat out uint currentRotation;
// ...
currentRotation = rotation;
```

---

## Future Work: Axis-Based Block Rotation (Logs)

Logs in Minecraft can be placed along different axes (vertical, horizontal X, horizontal Z) based on which face the player clicks. This is different from the horizontal facing rotation described above.

**Current limitation**: Oak logs are marked as `directional`, but they use the same horizontal rotation system. True log placement would require:

1. **Detecting clicked face**: Determine if player clicked a horizontal or vertical face
2. **Axis storage**: Store which axis the log is oriented along (Y, X, or Z)
3. **Mesh generation**: Rotate the entire block geometry, not just remap textures
4. **UV mapping**: Top/bottom faces become side faces when placed horizontally

This feature is **not yet implemented** and would require additional work beyond the current directional block system.

---

## Step 8: Fix Block ID Lookups

After packing rotation with block ID, update all places that read Material to extract just the block ID.

**Places to update**:

1. **BlockRegistry lookups** - Must use `getBlockId(material)`
2. **Air checks** - Air is still ID 0, but check `getBlockId(material) == 0`
3. **Block picking** (middle click) - Extract ID without rotation

**Pseudo code** for air check:

```
// OLD
if material == 0: // is air

// NEW
if getBlockId(material) == 0: // is air
```

**For isAirAtSnapshot**:

```
function isAirAtSnapshot(blockData, neighbors, x, y, z):
    material = ... // get from appropriate buffer
    return getBlockId(material) == 0
```

---

## Implementation Checklist

### Phase 1: Data Layer
- [ ] Add pack/unpack helpers to `Material.h` or new `BlockData.h`
- [ ] Add `isDirectional` field to `BlockMeta`
- [ ] Mark directional blocks (furnace, etc.) in registry

### Phase 2: Placement
- [ ] Add `getHorizontalFacing(yaw)` function
- [ ] Add `getOppositeFacing(facing)` function
- [ ] Modify `Player::handleInputs()` to pack rotation when placing

### Phase 3: Rendering
- [ ] Add face remapping function
- [ ] Update `buildMeshJob()` to extract rotation from material
- [ ] Update `buildMeshJob()` to remap faces for directional blocks
- [ ] Update air checks to use `getBlockId()`

### Phase 4: Polish
- [ ] Verify block picking (middle click) works correctly
- [ ] Test with multiple directional blocks
- [ ] (Optional) Add UV rotation in shader for top faces

---

## Example: Complete Flow

1. Player looks at ground (camera yaw = 180, facing NORTH/-Z)
2. Player right-clicks to place furnace
3. `getHorizontalFacing(180)` returns `NORTH`
4. `getOppositeFacing(NORTH)` returns `SOUTH` (block should face player)
5. `packBlockData(furnaceId, SOUTH=1)` creates packed material
6. Block stored in chunk with rotation = 1
7. During mesh generation:
   - Extract `blockId` and `rotation` from material
   - For world SOUTH face: `remapFaceForRotation(SOUTH, 1)` = `NORTH`
   - Fetch furnace's NORTH texture (the front face)
   - The front texture now appears on the world's SOUTH face
8. Player sees furnace front facing them

---

## File Summary

| File | Changes |
|------|---------|
| `Material.h` | Add pack/unpack functions, rotation constants |
| `BlockMeta` (in BlockRegistry) | Add `isDirectional` flag |
| `BlockRegistry.cpp` | Mark blocks as directional when registering |
| `Player.cpp` | Calculate and apply rotation during placement |
| `ChunkMeshManager.cpp` | Extract rotation, remap faces, fix air checks |
| `world.vert` (optional) | Already unpacks rotation, no changes needed |
| `world.frag` (optional) | Add UV rotation for top faces if desired |

---

## Memory Impact

**Option A (packed into Material)**: Zero additional memory
**Option B (separate array)**: +4KB per chunk (4096 bytes)

With Option A, you trade block ID range (65535 -> 8191) for zero overhead. This is an excellent tradeoff since 8191 block types is more than enough for most games.
