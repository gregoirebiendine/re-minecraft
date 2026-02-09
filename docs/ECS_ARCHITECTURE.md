# Entity Component System (ECS) Architecture

This document explains the ECS pattern and how it could be implemented in the re-minecraft engine.

---

## Table of Contents

1. [What is ECS?](#what-is-ecs)
2. [Core Concepts](#core-concepts)
3. [ECS vs Traditional OOP](#ecs-vs-traditional-oop)
4. [Benefits of ECS](#benefits-of-ecs)
5. [Implementation Approaches](#implementation-approaches)
6. [Theoretical Implementation for re-minecraft](#theoretical-implementation-for-re-minecraft)
7. [Migration Strategy](#migration-strategy)
8. [Popular ECS Libraries](#popular-ecs-libraries)

---

## What is ECS?

**Entity Component System (ECS)** is an architectural pattern primarily used in game development that favors **composition over inheritance**. It separates data (Components) from behavior (Systems), with Entities serving as identifiers that link components together.

The pattern emerged from the game development community to solve problems with traditional object-oriented hierarchies, which often lead to rigid class structures, poor cache performance, and difficulty in adding new features.

### The Three Pillars

```
┌─────────────────────────────────────────────────────────────────────┐
│                     ENTITY COMPONENT SYSTEM                         │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│   ┌──────────┐      ┌──────────────┐      ┌──────────────┐        │
│   │ ENTITY   │      │  COMPONENT   │      │    SYSTEM    │        │
│   │          │      │              │      │              │        │
│   │ Just an  │ has  │ Pure data    │ used │ Pure logic   │        │
│   │ ID/Handle│─────>│ No behavior  │ by   │ No state     │        │
│   │          │      │ No methods   │<─────│ Operates on  │        │
│   │          │      │              │      │ components   │        │
│   └──────────┘      └──────────────┘      └──────────────┘        │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## Core Concepts

### 1. Entity

An **Entity** is simply a unique identifier - nothing more than an ID (usually an integer or a lightweight handle). It has no data and no behavior by itself.

```cpp
// Simplest form - just an integer
using Entity = uint32_t;

// More sophisticated - a handle with generation for recycling
struct Entity {
    uint32_t id;         // Index into entity array
    uint32_t generation; // Incremented when entity is recycled
};
```

**Key Points:**
- Entities are created and destroyed dynamically
- They serve as "glue" that associates components together
- An entity with no components is essentially nothing
- Entity IDs can be recycled when entities are destroyed

### 2. Component

A **Component** is a plain data structure with no behavior (no methods that modify state). Components represent a single aspect or property of an entity.

```cpp
// Position in 3D space
struct PositionComponent {
    float x, y, z;
};

// Movement velocity
struct VelocityComponent {
    float vx, vy, vz;
};

// Physics properties
struct PhysicsComponent {
    float mass;
    float drag;
    bool isGrounded;
};

// Visual representation
struct RenderComponent {
    MeshId mesh;
    ShaderId shader;
    glm::vec4 color;
};

// Player input state
struct InputComponent {
    glm::vec3 moveDirection;
    bool jumpPressed;
    bool attackPressed;
};
```

**Key Points:**
- Components are **pure data** - no methods, no logic
- Each component represents a single responsibility
- Components are small and focused
- An entity can have any combination of components

### 3. System

A **System** contains all the logic and behavior. It operates on entities that have specific combinations of components.

```cpp
// Physics system - operates on entities with Position, Velocity, and Physics
class PhysicsSystem {
public:
    void update(float dt, World& ecs) {
        // Query all entities with required components
        for (auto [entity, pos, vel, phys] : ecs.query<Position, Velocity, Physics>()) {
            // Apply gravity
            if (!phys.isGrounded) {
                vel.vy -= 9.81f * dt;
            }

            // Apply drag
            vel.vx *= (1.0f - phys.drag * dt);
            vel.vy *= (1.0f - phys.drag * dt);
            vel.vz *= (1.0f - phys.drag * dt);

            // Update position
            pos.x += vel.vx * dt;
            pos.y += vel.vy * dt;
            pos.z += vel.vz * dt;
        }
    }
};
```

**Key Points:**
- Systems contain **all the logic**
- They query for entities with specific component combinations
- Systems are stateless (or have minimal state)
- Systems can run in parallel if they access different component types

### 4. World (Registry)

The **World** (also called Registry or EntityManager) is the central container that:
- Creates and destroys entities
- Attaches/detaches components to entities
- Provides query mechanisms for systems
- Manages component storage

```cpp
class World {
public:
    // Entity lifecycle
    Entity createEntity();
    void destroyEntity(Entity entity);
    bool isAlive(Entity entity);

    // Component operations
    template<typename T>
    T& addComponent(Entity entity, T component);

    template<typename T>
    void removeComponent(Entity entity);

    template<typename T>
    T& getComponent(Entity entity);

    template<typename T>
    bool hasComponent(Entity entity);

    // Querying
    template<typename... Components>
    View<Components...> query();
};
```

---

## ECS vs Traditional OOP

### Traditional OOP Approach (Inheritance)

```cpp
// The classic "diamond problem" and rigid hierarchy
class GameObject {
    glm::vec3 position;
    virtual void update(float dt) = 0;
    virtual void render() = 0;
};

class PhysicsObject : public GameObject {
    glm::vec3 velocity;
    CollisionBox collider;
};

class RenderableObject : public GameObject {
    Mesh mesh;
    Shader shader;
};

// What if we need both physics AND rendering?
// Multiple inheritance? Separate classes?
class Player : public PhysicsObject, public RenderableObject {
    // Diamond problem! Which position do we use?
    // Lots of boilerplate to resolve
};

// What about a particle that has physics but no collision?
// What about a trigger zone with collision but no physics?
// The hierarchy becomes a mess...
```

**Problems with OOP:**
1. **Rigid hierarchies** - Hard to change once established
2. **Diamond problem** - Multiple inheritance conflicts
3. **Feature explosion** - Every combination needs a new class
4. **Poor cache performance** - Objects scattered in memory
5. **Tight coupling** - Changes ripple through hierarchy

### ECS Approach (Composition)

```cpp
// Create entities and compose them with components
Entity player = world.createEntity();
world.addComponent(player, Position{0, 64, 0});
world.addComponent(player, Velocity{0, 0, 0});
world.addComponent(player, Physics{80.0f, 0.1f, false});
world.addComponent(player, Collision{/* AABB */});
world.addComponent(player, Render{playerMesh, defaultShader});
world.addComponent(player, PlayerInput{});
world.addComponent(player, Camera{90.0f, 0.0f, 0.0f});

// Particle with physics but no collision - just don't add Collision!
Entity particle = world.createEntity();
world.addComponent(particle, Position{10, 70, 10});
world.addComponent(particle, Velocity{1, 2, 0});
world.addComponent(particle, Physics{0.1f, 0.5f, false});
world.addComponent(particle, Render{particleMesh, particleShader});
// No Collision component = no collision!

// Trigger zone with collision but no physics
Entity trigger = world.createEntity();
world.addComponent(trigger, Position{50, 64, 50});
world.addComponent(trigger, Collision{/* large AABB */});
world.addComponent(trigger, TriggerZone{onPlayerEnter});
// No Physics, no Velocity = stationary trigger
```

**Benefits:**
1. **Flexible composition** - Any combination of components
2. **No inheritance issues** - Just data and IDs
3. **Easy to extend** - Add new components/systems without touching existing code
4. **Cache-friendly** - Components stored contiguously in memory
5. **Loose coupling** - Systems only know about components they need

---

## Benefits of ECS

### 1. Data-Oriented Design & Cache Performance

Components of the same type are stored contiguously in memory:

```
Traditional OOP (scattered memory):
┌──────────────────────────────────────────────────────────────────┐
│ Player1.pos │ garbage │ Enemy1.pos │ garbage │ Player1.vel │ ...│
└──────────────────────────────────────────────────────────────────┘
       ↑              ↑           ↑
    Cache miss    Cache miss   Cache miss (data all over memory)

ECS (contiguous arrays):
Position array: [Player1.pos][Enemy1.pos][Enemy2.pos][Particle.pos]...
                     ↓           ↓           ↓           ↓
                 All in cache! Sequential memory access = fast

Velocity array: [Player1.vel][Enemy1.vel][Enemy2.vel][Particle.vel]...
```

This can result in **10-100x performance improvements** for systems processing many entities.

### 2. Parallelization

Systems that operate on different component types can run in parallel:

```cpp
// These can run in parallel - no shared data!
std::thread t1([&]() { physicsSystem.update(dt, world); });  // Position, Velocity, Physics
std::thread t2([&]() { aiSystem.update(dt, world); });       // AI, Target
std::thread t3([&]() { animationSystem.update(dt, world); }); // Animation, Skeleton

// Wait for all to complete
t1.join(); t2.join(); t3.join();

// Rendering must wait (needs Position)
renderSystem.render(world);
```

### 3. Easy Addition of Features

Adding a new feature doesn't require modifying existing code:

```cpp
// Want to add a health system? Just create new component and system:

struct HealthComponent {
    int current;
    int maximum;
    float regenRate;
};

class HealthSystem {
public:
    void update(float dt, World& world) {
        for (auto [entity, health] : world.query<Health>()) {
            if (health.current < health.maximum) {
                health.current += health.regenRate * dt;
            }
        }
    }
};

// Existing entities can gain health by just adding the component:
world.addComponent(existingEnemy, Health{100, 100, 1.0f});
```

### 4. Runtime Flexibility

Entities can change their composition at runtime:

```cpp
// Player picks up a jetpack
world.addComponent(player, Jetpack{100.0f, 5.0f}); // fuel, thrust

// Player enters water - add swimming component
world.addComponent(player, Swimming{});
world.removeComponent<Walking>(player);

// Player dies - remove physics, add ragdoll
world.removeComponent<Physics>(player);
world.addComponent(player, Ragdoll{});
```

### 5. Serialization

Pure data components are trivial to serialize:

```cpp
void saveGame(World& world, std::ostream& out) {
    for (auto [entity, pos, vel] : world.query<Position, Velocity>()) {
        out << entity.id << " "
            << pos.x << " " << pos.y << " " << pos.z << " "
            << vel.vx << " " << vel.vy << " " << vel.vz << "\n";
    }
}
```

---

## Implementation Approaches

### Approach 1: Archetype-Based (Used by Unity DOTS, flecs)

Entities with the same component combination (archetype) are stored together:

```
Archetype: [Position, Velocity, Physics]
┌─────────────────────────────────────────────────────────┐
│ Entity IDs:    [1]    [5]    [12]   [15]   [23]        │
│ Positions:     [...]  [...]  [...]  [...]  [...]       │
│ Velocities:    [...]  [...]  [...]  [...]  [...]       │
│ Physics:       [...]  [...]  [...]  [...]  [...]       │
└─────────────────────────────────────────────────────────┘

Archetype: [Position, Velocity, Physics, PlayerInput]
┌─────────────────────────────────────────────────────────┐
│ Entity IDs:    [0]                                      │
│ Positions:     [...]                                    │
│ Velocities:    [...]                                    │
│ Physics:       [...]                                    │
│ PlayerInputs:  [...]                                    │
└─────────────────────────────────────────────────────────┘
```

**Pros:**
- Excellent cache locality
- Fast iteration over component combinations
- Memory efficient

**Cons:**
- Adding/removing components requires moving entity to different archetype
- More complex implementation

### Approach 2: Sparse Set (Used by EnTT)

Each component type has its own sparse set storage:

```
Position Storage (Sparse Set):
┌────────────────────────────────────────────────────────┐
│ Dense:  [pos0] [pos1] [pos2] [pos3]                    │
│ Sparse: [0: 0] [1: 1] [5: 2] [12: 3] [other: invalid] │
│ Entity: [0]    [1]    [5]    [12]                      │
└────────────────────────────────────────────────────────┘

Velocity Storage (Sparse Set):
┌────────────────────────────────────────────────────────┐
│ Dense:  [vel0] [vel1] [vel2]                           │
│ Sparse: [0: 0] [5: 1] [12: 2] [other: invalid]        │
│ Entity: [0]    [5]    [12]                             │
└────────────────────────────────────────────────────────┘
```

**Pros:**
- O(1) component add/remove
- Simpler implementation
- Fast single-component iteration

**Cons:**
- Multi-component queries less cache-friendly
- Higher memory overhead

### Approach 3: Simple Arrays (Educational/Small Scale)

Basic implementation using vectors and maps:

```cpp
class SimpleECS {
    uint32_t nextEntityId = 0;

    // Component storage - one vector per type
    std::vector<Position> positions;
    std::vector<Velocity> velocities;
    // ... more component vectors

    // Maps entity ID to component index
    std::unordered_map<Entity, size_t> positionIndex;
    std::unordered_map<Entity, size_t> velocityIndex;
    // ... more index maps

public:
    Entity createEntity() {
        return nextEntityId++;
    }

    void addPosition(Entity e, Position pos) {
        positionIndex[e] = positions.size();
        positions.push_back(pos);
    }

    Position& getPosition(Entity e) {
        return positions[positionIndex.at(e)];
    }
};
```

**Pros:**
- Very simple to understand
- Good for learning

**Cons:**
- Doesn't scale well
- Removal is complex (need to handle holes or swap-remove)

---

## Theoretical Implementation for re-minecraft

Based on the current architecture, here's how ECS could be implemented:

### Current Architecture Issues

1. **Player class is monolithic** - Contains position, velocity, camera, collision, GUI all bundled together
2. **Tight coupling** - Player depends directly on World
3. **No support for multiple entities** - Only one player, no mobs/items/projectiles
4. **Thread safety issues** - Data races between chunk generation, meshing, and rendering

### Proposed Component Definitions

```cpp
// ============================================
// TRANSFORM COMPONENTS
// ============================================

struct Transform {
    glm::vec3 position;
    glm::vec3 rotation;  // Euler angles (pitch, yaw, roll)
    glm::vec3 scale = {1, 1, 1};
};

// ============================================
// PHYSICS COMPONENTS
// ============================================

struct Velocity {
    glm::vec3 linear;
    glm::vec3 angular;
};

struct Gravity {
    float strength = 0.015f;      // Gravity per tick
    float terminalVelocity = 0.27f;
};

struct Friction {
    float ground = 0.8f;  // Multiplier when grounded
    float air = 0.98f;    // Multiplier when airborne
};

struct CollisionBox {
    glm::vec3 halfExtents;  // Half-size of AABB
    glm::vec3 offset;       // Offset from transform position
    bool isGrounded = false;
};

struct Movement {
    float acceleration = 0.04f;
    float maxSpeed = 0.075f;
    float jumpForce = 0.18f;
    glm::vec3 inputDirection;  // Normalized movement input
    int jumpBufferFrames = 0;
};

// ============================================
// RENDERING COMPONENTS
// ============================================

struct Camera {
    float fov = 90.0f;
    float nearPlane = 0.01f;
    float farPlane = 1000.0f;
    float yaw = 0.0f;
    float pitch = 0.0f;
    glm::vec3 eyeOffset = {0, 1.75f, 0};  // Offset from transform
};

struct MeshRenderer {
    uint32_t meshId;
    uint32_t shaderId;
    bool castsShadow = true;
};

struct SpriteRenderer {
    uint32_t textureId;
    glm::vec2 size;
    glm::vec4 color = {1, 1, 1, 1};
};

// ============================================
// GAMEPLAY COMPONENTS
// ============================================

struct PlayerController {
    float mouseSensitivity = 0.1f;
    uint16_t selectedBlockId = 1;
};

struct Health {
    int current;
    int maximum;
    float invincibilityTimer = 0.0f;
};

struct Inventory {
    std::array<ItemStack, 36> slots;
    uint8_t selectedSlot = 0;
};

// ============================================
// AI COMPONENTS (for future mobs)
// ============================================

struct AIBrain {
    enum class State { IDLE, WANDER, CHASE, FLEE, ATTACK };
    State currentState = State::IDLE;
    Entity target = INVALID_ENTITY;
    float thinkTimer = 0.0f;
};

struct PathFollower {
    std::vector<glm::ivec3> path;
    size_t currentNode = 0;
    float speed = 0.05f;
};

// ============================================
// LIFECYCLE COMPONENTS
// ============================================

struct Lifetime {
    float remaining;  // Seconds until destruction
};

struct DestroyOnCollision {};  // Tag component

struct Spawner {
    Entity prefab;
    float interval;
    float timer = 0.0f;
    int maxCount = 10;
};
```

### Proposed System Definitions

```cpp
// ============================================
// INPUT SYSTEM
// ============================================

class InputSystem {
public:
    void update(World& ecs, const Input& input) {
        // Process player input
        for (auto [entity, controller, movement, camera] :
             ecs.query<PlayerController, Movement, Camera>()) {

            // Mouse look
            camera.yaw += input.mouseDeltaX * controller.mouseSensitivity;
            camera.pitch -= input.mouseDeltaY * controller.mouseSensitivity;
            camera.pitch = glm::clamp(camera.pitch, -89.0f, 89.0f);

            // Movement direction
            glm::vec3 forward = calculateForward(camera.yaw);
            glm::vec3 right = calculateRight(camera.yaw);

            movement.inputDirection = glm::vec3(0);
            if (input.isKeyHeld(GLFW_KEY_W)) movement.inputDirection += forward;
            if (input.isKeyHeld(GLFW_KEY_S)) movement.inputDirection -= forward;
            if (input.isKeyHeld(GLFW_KEY_A)) movement.inputDirection -= right;
            if (input.isKeyHeld(GLFW_KEY_D)) movement.inputDirection += right;

            if (glm::length(movement.inputDirection) > 0.01f) {
                movement.inputDirection = glm::normalize(movement.inputDirection);
            }

            // Jump buffering
            if (input.isKeyPressed(GLFW_KEY_SPACE)) {
                movement.jumpBufferFrames = 5;
            }
        }
    }
};

// ============================================
// PHYSICS SYSTEM
// ============================================

class PhysicsSystem {
public:
    void update(World& ecs, float dt) {
        // Apply gravity
        for (auto [entity, velocity, gravity, collision] :
             ecs.query<Velocity, Gravity, CollisionBox>()) {

            if (!collision.isGrounded) {
                velocity.linear.y -= gravity.strength;
                velocity.linear.y = std::max(velocity.linear.y, -gravity.terminalVelocity);
            }
        }

        // Apply movement acceleration
        for (auto [entity, velocity, movement, collision] :
             ecs.query<Velocity, Movement, CollisionBox>()) {

            glm::vec3 targetVel = movement.inputDirection * movement.maxSpeed;
            velocity.linear.x += (targetVel.x - velocity.linear.x) * movement.acceleration;
            velocity.linear.z += (targetVel.z - velocity.linear.z) * movement.acceleration;

            // Jump
            if (movement.jumpBufferFrames > 0 && collision.isGrounded) {
                velocity.linear.y = movement.jumpForce;
                movement.jumpBufferFrames = 0;
            } else if (movement.jumpBufferFrames > 0) {
                movement.jumpBufferFrames--;
            }
        }

        // Apply friction
        for (auto [entity, velocity, friction, collision] :
             ecs.query<Velocity, Friction, CollisionBox>()) {

            float f = collision.isGrounded ? friction.ground : friction.air;
            velocity.linear.x *= f;
            velocity.linear.z *= f;
        }
    }
};

// ============================================
// COLLISION SYSTEM
// ============================================

class CollisionSystem {
    BlockWorld& blockWorld;  // Reference to voxel world

public:
    CollisionSystem(BlockWorld& world) : blockWorld(world) {}

    void update(World& ecs, float dt) {
        for (auto [entity, transform, velocity, collision] :
             ecs.query<Transform, Velocity, CollisionBox>()) {

            collision.isGrounded = false;

            // Resolve each axis separately for sliding
            // Y axis first (gravity)
            transform.position.y += velocity.linear.y;
            if (resolveAxisCollision(transform, collision, 1, blockWorld)) {
                if (velocity.linear.y < 0) {
                    collision.isGrounded = true;
                }
                velocity.linear.y = 0;
            }

            // X axis
            transform.position.x += velocity.linear.x;
            if (resolveAxisCollision(transform, collision, 0, blockWorld)) {
                velocity.linear.x = 0;
            }

            // Z axis
            transform.position.z += velocity.linear.z;
            if (resolveAxisCollision(transform, collision, 2, blockWorld)) {
                velocity.linear.z = 0;
            }
        }
    }

private:
    bool resolveAxisCollision(Transform& t, CollisionBox& c, int axis, BlockWorld& world) {
        // Get AABB bounds
        glm::vec3 min = t.position + c.offset - c.halfExtents;
        glm::vec3 max = t.position + c.offset + c.halfExtents;

        // Check all blocks in AABB
        for (int x = floor(min.x); x <= floor(max.x); x++) {
            for (int y = floor(min.y); y <= floor(max.y); y++) {
                for (int z = floor(min.z); z <= floor(max.z); z++) {
                    if (world.isSolid(x, y, z)) {
                        // Resolve penetration
                        // ... collision resolution logic
                        return true;
                    }
                }
            }
        }
        return false;
    }
};

// ============================================
// CAMERA SYSTEM
// ============================================

class CameraSystem {
public:
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    glm::vec3 cameraPosition;

    void update(World& ecs, float aspectRatio) {
        // Find entity with camera (usually just one - the player)
        for (auto [entity, transform, camera] :
             ecs.query<Transform, Camera>()) {

            cameraPosition = transform.position + camera.eyeOffset;

            glm::vec3 direction;
            direction.x = cos(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
            direction.y = sin(glm::radians(camera.pitch));
            direction.z = sin(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
            direction = glm::normalize(direction);

            viewMatrix = glm::lookAt(
                cameraPosition,
                cameraPosition + direction,
                glm::vec3(0, 1, 0)
            );

            projectionMatrix = glm::perspective(
                glm::radians(camera.fov),
                aspectRatio,
                camera.nearPlane,
                camera.farPlane
            );
        }
    }
};

// ============================================
// RENDER SYSTEM
// ============================================

class RenderSystem {
    Shader& shader;

public:
    RenderSystem(Shader& s) : shader(s) {}

    void render(World& ecs, const CameraSystem& camera) {
        shader.use();
        shader.setMat4("view", camera.viewMatrix);
        shader.setMat4("projection", camera.projectionMatrix);

        // Render all mesh renderers
        for (auto [entity, transform, renderer] :
             ecs.query<Transform, MeshRenderer>()) {

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, transform.position);
            model = glm::rotate(model, transform.rotation.y, glm::vec3(0, 1, 0));
            model = glm::rotate(model, transform.rotation.x, glm::vec3(1, 0, 0));
            model = glm::scale(model, transform.scale);

            shader.setMat4("model", model);

            // meshManager.render(renderer.meshId);
        }
    }
};

// ============================================
// LIFETIME SYSTEM
// ============================================

class LifetimeSystem {
public:
    void update(World& ecs, float dt) {
        std::vector<Entity> toDestroy;

        for (auto [entity, lifetime] : ecs.query<Lifetime>()) {
            lifetime.remaining -= dt;
            if (lifetime.remaining <= 0) {
                toDestroy.push_back(entity);
            }
        }

        for (Entity e : toDestroy) {
            ecs.destroyEntity(e);
        }
    }
};

// ============================================
// AI SYSTEM (Future expansion)
// ============================================

class AISystem {
public:
    void update(World& ecs, float dt) {
        for (auto [entity, brain, transform, path] :
             ecs.query<AIBrain, Transform, PathFollower>()) {

            brain.thinkTimer -= dt;

            if (brain.thinkTimer <= 0) {
                brain.thinkTimer = 0.5f;  // Think every 0.5 seconds

                switch (brain.currentState) {
                    case AIBrain::State::IDLE:
                        // Maybe start wandering
                        if (rand() % 100 < 30) {
                            brain.currentState = AIBrain::State::WANDER;
                        }
                        break;

                    case AIBrain::State::WANDER:
                        // Random pathfinding
                        // path.path = findRandomPath(transform.position);
                        break;

                    case AIBrain::State::CHASE:
                        // Path toward target
                        // if (ecs.isAlive(brain.target)) {
                        //     auto& targetPos = ecs.get<Transform>(brain.target);
                        //     path.path = findPath(transform.position, targetPos.position);
                        // }
                        break;
                }
            }
        }
    }
};
```

### Main Game Loop with ECS

```cpp
class Game {
    // ECS World
    World ecs;

    // Block world (chunks, terrain)
    BlockWorld blockWorld;

    // Systems
    InputSystem inputSystem;
    PhysicsSystem physicsSystem;
    CollisionSystem collisionSystem;
    CameraSystem cameraSystem;
    RenderSystem renderSystem;
    LifetimeSystem lifetimeSystem;

    // The player entity
    Entity player;

public:
    void initialize() {
        // Create player entity with components
        player = ecs.createEntity();

        ecs.addComponent(player, Transform{
            .position = {0, 100, 0},
            .rotation = {0, 0, 0}
        });

        ecs.addComponent(player, Velocity{
            .linear = {0, 0, 0}
        });

        ecs.addComponent(player, Gravity{
            .strength = 0.015f,
            .terminalVelocity = 0.27f
        });

        ecs.addComponent(player, Friction{
            .ground = 0.8f,
            .air = 0.98f
        });

        ecs.addComponent(player, CollisionBox{
            .halfExtents = {0.3f, 0.9f, 0.3f},
            .offset = {0, 0.9f, 0}
        });

        ecs.addComponent(player, Movement{
            .acceleration = 0.04f,
            .maxSpeed = 0.075f,
            .jumpForce = 0.18f
        });

        ecs.addComponent(player, Camera{
            .fov = 90.0f,
            .eyeOffset = {0, 1.75f, 0}
        });

        ecs.addComponent(player, PlayerController{
            .mouseSensitivity = 0.1f
        });
    }

    void update(float dt, const Input& input) {
        // Fixed timestep accumulator
        static float accumulator = 0;
        accumulator += dt;

        constexpr float FIXED_DT = 1.0f / 60.0f;

        while (accumulator >= FIXED_DT) {
            // Run systems in order
            inputSystem.update(ecs, input);
            physicsSystem.update(ecs, FIXED_DT);
            collisionSystem.update(ecs, FIXED_DT);
            lifetimeSystem.update(ecs, FIXED_DT);

            accumulator -= FIXED_DT;
        }

        // Camera can update every frame for smoothness
        cameraSystem.update(ecs, aspectRatio);
    }

    void render() {
        // Update chunk streaming based on camera position
        blockWorld.updateStreaming(cameraSystem.cameraPosition);

        // Render chunks
        blockWorld.render(cameraSystem);

        // Render entities
        renderSystem.render(ecs, cameraSystem);
    }
};
```

### Creating Other Entities

```cpp
// Spawn a zombie mob
Entity zombie = ecs.createEntity();
ecs.addComponent(zombie, Transform{{100, 65, 100}});
ecs.addComponent(zombie, Velocity{});
ecs.addComponent(zombie, Gravity{0.015f, 0.27f});
ecs.addComponent(zombie, Friction{0.8f, 0.98f});
ecs.addComponent(zombie, CollisionBox{{0.3f, 0.9f, 0.3f}, {0, 0.9f, 0}});
ecs.addComponent(zombie, Movement{0.02f, 0.04f, 0.12f});  // Slower than player
ecs.addComponent(zombie, Health{20, 20});
ecs.addComponent(zombie, AIBrain{AIBrain::State::WANDER});
ecs.addComponent(zombie, PathFollower{});
ecs.addComponent(zombie, MeshRenderer{zombieMeshId, entityShaderId});

// Spawn a falling block (sand/gravel)
Entity fallingBlock = ecs.createEntity();
ecs.addComponent(fallingBlock, Transform{{50, 80, 50}});
ecs.addComponent(fallingBlock, Velocity{{0, -0.1f, 0}});
ecs.addComponent(fallingBlock, Gravity{0.01f, 0.5f});
ecs.addComponent(fallingBlock, MeshRenderer{sandBlockMeshId, blockShaderId});
ecs.addComponent(fallingBlock, DestroyOnCollision{});  // Tag: destroy when hits ground

// Spawn a particle
Entity particle = ecs.createEntity();
ecs.addComponent(particle, Transform{{playerPos + glm::vec3(0, 1, 0)}});
ecs.addComponent(particle, Velocity{{randFloat(-1, 1), randFloat(0.5f, 2), randFloat(-1, 1)}});
ecs.addComponent(particle, Gravity{0.02f, 1.0f});
ecs.addComponent(particle, SpriteRenderer{smokeTextureId, {0.2f, 0.2f}});
ecs.addComponent(particle, Lifetime{2.0f});  // Dies after 2 seconds

// Spawn dropped item
Entity droppedItem = ecs.createEntity();
ecs.addComponent(droppedItem, Transform{{blockPos + glm::vec3(0.5f)}});
ecs.addComponent(droppedItem, Velocity{{randFloat(-0.1f, 0.1f), 0.15f, randFloat(-0.1f, 0.1f)}});
ecs.addComponent(droppedItem, Gravity{0.01f, 0.3f});
ecs.addComponent(droppedItem, CollisionBox{{0.125f, 0.125f, 0.125f}});
ecs.addComponent(droppedItem, SpriteRenderer{itemTextureId, {0.25f, 0.25f}});
ecs.addComponent(droppedItem, Lifetime{300.0f});  // Despawn after 5 minutes
ecs.addComponent(droppedItem, Pickupable{ITEM_COBBLESTONE, 1});
```

### Entity Factory Functions

The examples above show the "raw" way of creating entities, but this approach has drawbacks:
- Repetitive code when spawning the same entity type multiple times
- Easy to forget a required component
- Hard to maintain if component requirements change

**Factory functions** encapsulate entity creation, ensuring entities are always created with the correct set of components:

```cpp
// ============================================
// ENTITY FACTORIES
// ============================================

namespace EntityFactory {

    // Create the player entity
    Entity createPlayer(World& ecs, glm::vec3 position) {
        Entity player = ecs.createEntity();

        ecs.addComponent(player, Transform{
            .position = position,
            .rotation = {0, 0, 0}
        });
        ecs.addComponent(player, Velocity{});
        ecs.addComponent(player, Gravity{0.015f, 0.27f});
        ecs.addComponent(player, Friction{0.8f, 0.98f});
        ecs.addComponent(player, CollisionBox{{0.3f, 0.9f, 0.3f}, {0, 0.9f, 0}});
        ecs.addComponent(player, Movement{0.04f, 0.075f, 0.18f});
        ecs.addComponent(player, Camera{90.0f, 0.01f, 1000.0f});
        ecs.addComponent(player, PlayerController{0.1f});
        ecs.addComponent(player, Health{20, 20});

        return player;
    }

    // Create a zombie mob
    Entity createZombie(World& ecs, glm::vec3 position) {
        Entity zombie = ecs.createEntity();

        ecs.addComponent(zombie, Transform{position});
        ecs.addComponent(zombie, Velocity{});
        ecs.addComponent(zombie, Gravity{0.015f, 0.27f});
        ecs.addComponent(zombie, Friction{0.8f, 0.98f});
        ecs.addComponent(zombie, CollisionBox{{0.3f, 0.9f, 0.3f}, {0, 0.9f, 0}});
        ecs.addComponent(zombie, Movement{0.02f, 0.04f, 0.12f});  // Slower than player
        ecs.addComponent(zombie, Health{20, 20});
        ecs.addComponent(zombie, AIBrain{AIBrain::State::WANDER});
        ecs.addComponent(zombie, PathFollower{});
        ecs.addComponent(zombie, MeshRenderer{MESH_ZOMBIE, SHADER_ENTITY});

        return zombie;
    }

    // Create a skeleton mob
    Entity createSkeleton(World& ecs, glm::vec3 position) {
        Entity skeleton = ecs.createEntity();

        ecs.addComponent(skeleton, Transform{position});
        ecs.addComponent(skeleton, Velocity{});
        ecs.addComponent(skeleton, Gravity{0.015f, 0.27f});
        ecs.addComponent(skeleton, Friction{0.8f, 0.98f});
        ecs.addComponent(skeleton, CollisionBox{{0.3f, 0.9f, 0.3f}, {0, 0.9f, 0}});
        ecs.addComponent(skeleton, Movement{0.025f, 0.05f, 0.12f});
        ecs.addComponent(skeleton, Health{20, 20});
        ecs.addComponent(skeleton, AIBrain{AIBrain::State::WANDER});
        ecs.addComponent(skeleton, PathFollower{});
        ecs.addComponent(skeleton, MeshRenderer{MESH_SKELETON, SHADER_ENTITY});

        return skeleton;
    }

    // Create a falling block (sand, gravel)
    Entity createFallingBlock(World& ecs, glm::vec3 position, uint32_t blockMeshId) {
        Entity block = ecs.createEntity();

        ecs.addComponent(block, Transform{position});
        ecs.addComponent(block, Velocity{{0, -0.1f, 0}});
        ecs.addComponent(block, Gravity{0.01f, 0.5f});
        ecs.addComponent(block, MeshRenderer{blockMeshId, SHADER_BLOCK});
        ecs.addComponent(block, DestroyOnCollision{});

        return block;
    }

    // Create a particle effect
    Entity createParticle(World& ecs, glm::vec3 position, uint32_t textureId,
                          float lifetime = 2.0f) {
        Entity particle = ecs.createEntity();

        ecs.addComponent(particle, Transform{position});
        ecs.addComponent(particle, Velocity{{
            randFloat(-1.0f, 1.0f),
            randFloat(0.5f, 2.0f),
            randFloat(-1.0f, 1.0f)
        }});
        ecs.addComponent(particle, Gravity{0.02f, 1.0f});
        ecs.addComponent(particle, SpriteRenderer{textureId, {0.2f, 0.2f}});
        ecs.addComponent(particle, Lifetime{lifetime});

        return particle;
    }

    // Create a dropped item
    Entity createDroppedItem(World& ecs, glm::vec3 position, uint16_t itemId,
                             uint8_t count = 1) {
        Entity item = ecs.createEntity();

        ecs.addComponent(item, Transform{position});
        ecs.addComponent(item, Velocity{{
            randFloat(-0.1f, 0.1f),
            0.15f,
            randFloat(-0.1f, 0.1f)
        }});
        ecs.addComponent(item, Gravity{0.01f, 0.3f});
        ecs.addComponent(item, CollisionBox{{0.125f, 0.125f, 0.125f}});
        ecs.addComponent(item, SpriteRenderer{getItemTexture(itemId), {0.25f, 0.25f}});
        ecs.addComponent(item, Lifetime{300.0f});  // 5 minutes
        ecs.addComponent(item, Pickupable{itemId, count});

        return item;
    }

    // Create a projectile (arrow)
    Entity createArrow(World& ecs, glm::vec3 position, glm::vec3 direction,
                       float speed = 1.5f) {
        Entity arrow = ecs.createEntity();

        ecs.addComponent(arrow, Transform{position});
        ecs.addComponent(arrow, Velocity{glm::normalize(direction) * speed});
        ecs.addComponent(arrow, Gravity{0.005f, 2.0f});
        ecs.addComponent(arrow, CollisionBox{{0.05f, 0.05f, 0.25f}});
        ecs.addComponent(arrow, MeshRenderer{MESH_ARROW, SHADER_ENTITY});
        ecs.addComponent(arrow, Lifetime{60.0f});  // 1 minute
        ecs.addComponent(arrow, DestroyOnCollision{});
        ecs.addComponent(arrow, Damage{4});

        return arrow;
    }

} // namespace EntityFactory
```

**Usage:**

```cpp
// Clean and simple entity creation
Entity player = EntityFactory::createPlayer(ecs, {0, 100, 0});

// Spawn mobs
Entity zombie = EntityFactory::createZombie(ecs, {100, 65, 100});
Entity skeleton = EntityFactory::createSkeleton(ecs, {120, 65, 80});

// Spawn effects
EntityFactory::createParticle(ecs, playerPos + glm::vec3(0, 1, 0), TEXTURE_SMOKE);

// Drop items
EntityFactory::createDroppedItem(ecs, blockPos, ITEM_COBBLESTONE, 1);
```

**Benefits of Factory Functions:**
- **Single source of truth** - Component requirements defined in one place
- **Type-safe** - Compiler catches missing parameters
- **Easy to modify** - Change the factory, all entities update
- **Self-documenting** - Function signature shows required parameters
- **Encapsulation** - Hide internal component details from callers

---

## Migration Strategy

### Phase 1: Introduce ECS Alongside Current Code

1. Add an ECS library (EnTT recommended)
2. Create basic component structs mirroring current data
3. Create player entity with components
4. Systems initially just delegate to existing code

### Phase 2: Migrate Systems One by One

1. **Input System** - Extract from Player::handleInputs
2. **Physics System** - Extract gravity, friction, movement
3. **Collision System** - Extract from CollisionBox
4. **Camera System** - Extract from Camera class

### Phase 3: Add New Features Using ECS

1. Implement mobs as entities
2. Add particles as entities
3. Item drops as entities
4. Projectiles (arrows) as entities

### Phase 4: Remove Legacy Code

1. Delete old Player monolithic class
2. Delete old Camera class (replaced by component + system)
3. All entities flow through ECS

---

## Popular ECS Libraries

### EnTT (Recommended for C++)

```cpp
#include <entt/entt.hpp>

entt::registry registry;

// Create entity
auto entity = registry.create();

// Add components
registry.emplace<Position>(entity, 0.0f, 0.0f, 0.0f);
registry.emplace<Velocity>(entity, 1.0f, 0.0f, 0.0f);

// Query and iterate
auto view = registry.view<Position, Velocity>();
for (auto [entity, pos, vel] : view.each()) {
    pos.x += vel.x * dt;
}
```

**Pros:**
- Header-only, easy to integrate
- Very fast (one of the fastest ECS implementations)
- Well documented
- Active development

**GitHub:** https://github.com/skypjack/entt

### flecs (Alternative)

```cpp
#include <flecs.h>

flecs::world world;

// Create entity with components
auto e = world.entity()
    .set<Position>({0, 0, 0})
    .set<Velocity>({1, 0, 0});

// Create system
world.system<Position, const Velocity>()
    .each([](Position& p, const Velocity& v) {
        p.x += v.x;
    });

// Run systems
world.progress();
```

**Pros:**
- Built-in scheduler
- Relationship support
- Query caching
- Great for complex simulations

**GitHub:** https://github.com/SanderMertens/flecs

---

## Summary

ECS provides a powerful architectural pattern that would solve several issues in the current re-minecraft codebase:

| Current Problem | ECS Solution |
|-----------------|--------------|
| Monolithic Player class | Decompose into focused components |
| Hard to add mobs/items | Just create entities with different component combinations |
| Tight coupling | Systems only know about components |
| Thread safety issues | Systems can be parallelized by component access patterns |
| Poor extensibility | New features = new components + systems |

The recommended approach is to **adopt EnTT** and gradually migrate existing functionality while adding new features exclusively through ECS.
