# Gravity in Video Games

This guide explains the fundamental concepts of gravity simulation in video games, particularly for platformers and voxel-based games like Minecraft.

## Core Concept

Gravity in games is a **constant downward acceleration** applied to entities every frame or tick. Unlike real-world physics, game gravity is simplified to achieve responsive, predictable gameplay rather than realistic simulation.

## The Three Key Variables

### 1. Position

The entity's location in 3D space. Gravity indirectly affects position by modifying velocity, which then modifies position.

- Stored as a vector (x, y, z)
- Y-axis typically represents vertical height
- Updated every frame based on current velocity

### 2. Velocity

The rate of change of position. This is what gravity directly modifies.

- Also stored as a vector (x, y, z)
- Gravity only affects the Y component (vertical velocity)
- Positive Y = moving upward, Negative Y = moving downward
- Accumulates over time until countered by collision or terminal velocity

### 3. Acceleration (Gravity Constant)

A fixed value representing gravitational pull. In most games, this is a single constant rather than a calculated force.

- Typical values range from 0.01 to 1.0 depending on scale and desired feel
- Applied every tick/frame to the velocity
- Minecraft uses approximately 0.08 blocks per tick squared

## The Update Loop

Every game tick, gravity simulation follows this sequence:

1. **Apply gravity to velocity** - Decrease vertical velocity by the gravity constant
2. **Cap velocity** - Ensure velocity doesn't exceed terminal velocity
3. **Apply velocity to position** - Move the entity based on current velocity
4. **Check collisions** - Detect if entity hit ground or ceiling
5. **Resolve collisions** - Stop movement, reset velocity if grounded

## Terminal Velocity

In real life, air resistance eventually balances gravitational acceleration, creating a maximum falling speed. Games simulate this with a simple cap.

- Prevents entities from falling infinitely fast
- Makes falling feel consistent regardless of height
- Minecraft caps falling speed at approximately 3.92 blocks per tick

## Ground Detection

An entity is considered "grounded" when:

- There is a solid block/surface directly below
- The distance to that surface is within a small threshold (epsilon)
- The entity's vertical velocity is zero or downward

When grounded:
- Vertical velocity is typically reset to zero
- The entity is allowed to jump
- Gravity still applies but is immediately countered by collision

## Jumping

Jumping is simply applying an **instantaneous upward velocity**. Gravity then naturally brings the entity back down.

- Jump strength is a single impulse value added to vertical velocity
- Higher jump = larger initial velocity
- Gravity immediately begins counteracting the jump
- The arc of a jump is a natural parabola

### Jump Variables

- **Jump velocity** - The initial upward speed when jump begins
- **Jump buffer** - Short window allowing jump input slightly before landing
- **Coyote time** - Short window allowing jump slightly after leaving a ledge
- **Double jump** - Resetting or adding velocity while airborne

## Variable Jump Height

Many games allow higher jumps when holding the jump button longer. This is achieved by:

- Applying reduced gravity while jump is held and entity is rising
- Or cutting vertical velocity when jump button is released early
- Creates more responsive, skill-based movement

## The Relationship Between Variables

```
Frame N:
    velocity.y = velocity.y - gravity
    velocity.y = max(velocity.y, -terminalVelocity)
    position.y = position.y + velocity.y

Frame N+1:
    (repeat)
```

Each frame, gravity reduces vertical velocity. Velocity then changes position. This creates the characteristic parabolic arc of jumping and falling.

## Common Values and Ratios

The "feel" of gravity depends on the ratio between:

- **Gravity constant** - How fast you accelerate downward
- **Jump velocity** - How fast you initially move upward
- **Terminal velocity** - Maximum falling speed
- **Tick rate** - How often physics updates

A higher gravity-to-jump ratio creates snappy, responsive jumps. A lower ratio creates floaty, moon-like movement.

## Minecraft-Specific Behavior

Minecraft's gravity system has these characteristics:

- Gravity is applied per tick (20 ticks per second)
- Falling entities accelerate gradually
- Landing from high falls causes damage based on distance fallen
- Swimming and flying have modified or disabled gravity
- Creative mode flight completely overrides normal gravity

## Common Pitfalls

### Frame-Rate Independence

If gravity is applied per frame rather than per fixed time step, entities fall faster on higher refresh rates. Solutions:

- Use fixed timestep physics updates
- Multiply gravity by delta time

### Tunneling

Fast-falling entities can pass through thin platforms. Solutions:

- Use swept collision detection
- Limit maximum velocity
- Check collisions at multiple points along the movement path

### Floating Point Accumulation

Small position adjustments can accumulate errors over time. Solutions:

- Snap to ground when grounded
- Use fixed-point arithmetic for deterministic physics
- Reset velocity to exactly zero when landing

## Summary

Gravity in games is fundamentally simple: subtract a constant from vertical velocity each tick, then add velocity to position. The complexity comes from collision response, jump mechanics, and special cases. The key to good-feeling gravity is tuning the constants until movement feels responsive and predictable.
