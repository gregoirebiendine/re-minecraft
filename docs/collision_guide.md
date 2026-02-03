# 3D Collisions in Voxel Games

This guide explains the fundamental concepts of 3D collision detection and response in voxel-based games like Minecraft.

## Core Concept

Collision detection in voxel games answers two questions: **"Is there something solid where I want to move?"** and **"Which surface did I hit?"**. Unlike physics engines with complex polygon meshes, voxel worlds simplify collision to checking discrete block positions on a 3D grid.

## Collision Detection Methods

### Ray Casting

Ray casting is the primary collision detection method. It simulates a line (ray) traveling through space and reports what it hits.

**How it works:**

1. Start at an origin point with a direction vector
2. Move along the ray in small increments (steps)
3. At each step, check if a solid block exists at that position
4. If solid, return hit information (position, face hit, previous position)
5. If maximum distance reached without hitting, return no hit

**Key parameters:**

- **Origin** - Starting point of the ray (e.g., player position)
- **Direction** - Normalized vector indicating ray direction
- **Step size** - How far to move each iteration (smaller = more precise, slower)
- **Max distance** - How far to cast before giving up

Ray casting is efficient for:
- Determining what the player is looking at
- Ground detection (cast downward from player)
- Line-of-sight checks

### AABB (Axis-Aligned Bounding Box)

AABB collision treats entities as rectangular boxes aligned with world axes. While not currently implemented for player collision in this codebase, it's the standard approach for entity-world collision.

**Concept:**

- Entity is enclosed in an invisible box defined by min/max corners
- Check if any blocks overlap with this box
- Resolve by pushing the entity out of solid blocks

**Advantages over point collision:**

- Accounts for entity width and height
- Prevents clipping through walls when moving sideways
- More realistic collision shape

## Player AABB Dimensions

In Minecraft, the player's bounding box has specific dimensions that define their physical presence in the world.

### Standard Player Dimensions

| State     | Width  | Height | Depth  |
|-----------|--------|--------|--------|
| Standing  | 0.6    | 1.8    | 0.6    |
| Sneaking  | 0.6    | 1.5    | 0.6    |
| Swimming  | 0.6    | 0.6    | 0.6    |

The player is **0.6 blocks wide** (same on X and Z axes) and **1.8 blocks tall** when standing.

### Position Reference Point

The player's position typically refers to their **feet** (bottom center of the bounding box). This means:

- Position Y = bottom of the player
- Position X, Z = center of the player horizontally

### Calculating AABB Corners

Given a player position, the bounding box corners are:

```
half_width = 0.3  (0.6 / 2)
height = 1.8

min_corner = (position.x - half_width, position.y, position.z - half_width)
max_corner = (position.x + half_width, position.y + height, position.z + half_width)
```

### Visual Representation

```
        Top View (looking down)              Side View

             0.6 blocks                      +-----+ max_corner
           +----------+                      |     |
           |          |                      |     | 1.8
           |    +     | 0.6 blocks           |     | blocks
           | (pos.xz) |                      |     |
           +----------+                      +--+--+ position (feet)
                                               |
                                            (pos.y)
```

### Eye Position

The camera/eye position is offset from the feet position:

- **Standing eye height**: ~1.62 blocks above feet
- **Sneaking eye height**: ~1.27 blocks above feet

This is why raycasts for block selection originate from `position.y + eye_height`, not from the feet.

### AABB vs Block Overlap

To check if a player collides with blocks, you must check **all blocks that the AABB overlaps**:

```
For a player at position (10.5, 65.0, 20.5):

min_corner = (10.2, 65.0, 20.2)
max_corner = (10.8, 66.8, 20.8)

Blocks to check:
- X: floor(10.2) to floor(10.8) = blocks 10
- Y: floor(65.0) to floor(66.8) = blocks 65, 66
- Z: floor(20.2) to floor(20.8) = blocks 20

Total: 2 blocks to check (10, 65, 20) and (10, 66, 20)
```

When the player straddles block boundaries (e.g., position 10.0), more blocks must be checked:

```
For a player at position (10.0, 65.0, 20.0):

min_corner = (9.7, 65.0, 19.7)
max_corner = (10.3, 66.8, 20.3)

Blocks to check:
- X: blocks 9, 10
- Y: blocks 65, 66
- Z: blocks 19, 20

Total: 8 blocks to check (2 x 2 x 2 grid)
```

### Collision Resolution with AABB

When the player's AABB overlaps a solid block:

1. **Calculate penetration depth** on each axis
2. **Push out on the axis with smallest penetration** (separating axis theorem)
3. **Zero velocity** on that axis
4. **Repeat** until no overlaps remain

This prevents the player from walking through walls or falling through floors.

## Ground Detection

Ground detection determines if an entity is standing on a solid surface. This is critical for:

- Enabling/disabling gravity
- Allowing jumps
- Resetting vertical velocity

### The Approach

Cast a ray downward from the entity's position with a maximum distance slightly greater than the entity's height. If the ray hits a block within that distance, the entity is grounded.

**Key considerations:**

- **Ray origin** - Cast from the entity's feet position
- **Max distance** - Should match expected standing height plus small epsilon
- **Previous position** - The air block just above the solid block (used for position snapping)

### Grounding Response

When grounded:

1. If falling (negative vertical velocity), reset velocity to zero
2. Snap position to the top of the solid block
3. Enable jump capability

The position snap is calculated as: `previous_hit_position.y + entity_height`

This ensures the entity rests exactly on top of the block rather than floating or sinking.

## Block Face Detection

When a ray hits a block, knowing which face was hit is essential for:

- Placing blocks on the correct side
- Applying directional damage
- Determining collision response direction

### The Six Faces

Blocks in voxel games have six faces:

| Face   | Normal Direction | Triggered When       |
|--------|------------------|----------------------|
| UP     | (0, 1, 0)        | Ray comes from above |
| DOWN   | (0, -1, 0)       | Ray comes from below |
| NORTH  | (0, 0, -1)       | Ray comes from -Z    |
| SOUTH  | (0, 0, 1)        | Ray comes from +Z    |
| WEST   | (-1, 0, 0)       | Ray comes from -X    |
| EAST   | (1, 0, 0)        | Ray comes from +X    |

### Calculating Hit Face

The hit face is determined by comparing the current block position with the previous position (last air block):

```
difference = current_position - previous_position

if difference.x == 1  -> hit WEST face
if difference.x == -1 -> hit EAST face
if difference.y == 1  -> hit DOWN face
if difference.y == -1 -> hit UP face
if difference.z == 1  -> hit NORTH face
if difference.z == -1 -> hit SOUTH face
```

## Block Query System

Collision detection relies on efficient block lookup. The world provides simple query functions:

- **Get block at position** - Returns the material/type at world coordinates
- **Is position solid** - Returns whether a block exists (not air)
- **Is position air** - Returns whether position is empty

These queries handle:

- Converting world coordinates to chunk coordinates
- Loading/accessing the correct chunk data
- Returning default values for unloaded regions (typically air)

## Integration with Physics

Collision detection integrates with the physics loop in a specific order:

### Per-Tick Update Sequence

1. **Check collisions first** - Determine current grounding state
2. **Apply gravity** - Only if not grounded
3. **Handle jump input** - Only if grounded
4. **Apply horizontal movement** - Acceleration, friction, speed cap
5. **Update position** - Add velocity to position
6. **Snap if needed** - Correct position based on collision

### Why Order Matters

Checking collision before applying physics ensures:

- Gravity doesn't accumulate while standing on ground
- Jump only triggers when actually grounded
- Position corrections happen immediately upon landing

## World Interaction Raycasting

Beyond movement collision, ray casting enables world interaction:

### Block Selection

Cast a ray from the camera position in the view direction. The first solid block hit is the "selected" block that can be broken or modified.

### Block Placement

When placing a block, use the **previous position** from the raycast hit. This is the air block adjacent to the solid block, on the face the player is looking at.

**Example:**
- Player looks at a block and raycast hits position (5, 10, 3)
- Previous position was (5, 11, 3) - the air block above
- New block is placed at (5, 11, 3)

## Common Collision Scenarios

### Standing and Walking

- Continuous downward raycast checks for ground
- Position snaps to top of blocks
- Horizontal movement doesn't check wall collision (point-based limitation)

### Falling and Landing

- Ground check returns no hit while falling
- Gravity accumulates on vertical velocity
- Upon landing, raycast hits ground, velocity resets, position snaps

### Jumping

- Jump only allowed when ground raycast hits
- Instant upward velocity applied
- Gravity naturally brings entity back down
- Landing detected when downward raycast hits again

## Limitations of Point-Based Collision

Current architecture treats the player as a single point for collision. This has trade-offs:

### Advantages

- Simple implementation
- Fast computation
- Sufficient for basic voxel gameplay
- Easy to debug

### Limitations

- **No wall collision** - Player can clip into walls horizontally
- **No ceiling collision** - Player can jump through single-block ceilings
- **Edge cases** - Standing on block edges may behave unexpectedly
- **Width ignored** - Player has no physical width

### Future Improvements

A more complete system would use:

- **Multi-point raycasting** - Cast from multiple corners of player bounding box
- **Full AABB collision** - Check overlap with all nearby blocks
- **Swept collision** - Check collision along entire movement path

## Common Pitfalls

### Tunneling

Fast-moving entities can pass through thin walls if their velocity exceeds wall thickness per tick.

**Solutions:**
- Limit maximum velocity
- Use swept collision detection
- Check multiple points along movement path

### Floating Point Precision

Small position adjustments can accumulate errors, causing entities to slowly sink or rise.

**Solutions:**
- Snap to exact block boundaries when grounded
- Reset velocity to exactly zero (not near-zero)
- Use consistent position corrections

### Step Size Trade-offs

Smaller ray step sizes increase precision but cost performance. Larger steps may miss thin features.

**Solutions:**
- Use appropriate step size for use case (0.05 blocks is common)
- Consider DDA algorithm for perfect grid traversal
- Adjust based on expected velocities

### Chunk Boundaries

Block queries may fail at chunk boundaries if chunks aren't loaded.

**Solutions:**
- Return default value (air) for unloaded regions
- Prevent entity movement into unloaded areas
- Ensure neighboring chunks are loaded before physics

## Voxel-Specific Advantages

Voxel worlds simplify collision compared to arbitrary 3D geometry:

- **Discrete positions** - Blocks exist at integer coordinates
- **Uniform size** - All blocks are the same size (1x1x1)
- **Axis alignment** - No rotated collision boxes
- **Simple queries** - O(1) lookup for any position

These properties make ray casting through a voxel world efficient and predictable.

## Summary

3D collision in voxel games is fundamentally about checking discrete block positions. Ray casting provides an efficient method for ground detection and world interaction. The key steps are: cast a ray, check block existence at each step, and respond to hits by adjusting velocity and position. While point-based collision has limitations, it provides a solid foundation for basic voxel gameplay. More sophisticated systems add AABB collision and multi-point checks for complete entity-world interaction.
