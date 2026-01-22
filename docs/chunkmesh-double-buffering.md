# ChunkMesh Double Buffering & Remesh State Fix

## Overview

This document describes the fixes implemented to eliminate visual "blinking" artifacts when chunks are remeshed during block placement or destruction.

## Problem

When a block is placed or broken, the chunk would "blink" (disappear for one or more frames). This was caused by **two issues**:

### Issue 1: VAO/VBO Overwrite Race

With a single VAO/VBO, the old mesh data is overwritten via `glBufferData()` while the GPU may still be rendering from it, causing torn/partial geometry.

### Issue 2: Chunk State Filtering (Main Cause)

When a block was modified, the chunk state was changed from `READY` to `GENERATED`:

```cpp
// OLD CODE - caused blinking
chunk->setState(ChunkState::GENERATED);
```

But `getRenderableChunks()` only returns `READY` chunks:

```cpp
if (c.getState() == ChunkState::READY)
    out.push_back(&c);
```

This meant the chunk was **completely excluded from rendering** during the entire remeshing process (`GENERATED → MESHING → MESHED → READY`), causing the blink.

## Solution

### Fix 1: Double-Buffered VAO/VBO

The `ChunkMesh` class now maintains **two VAO/VBO pairs**:

- **Front buffer**: Currently being rendered
- **Back buffer**: Receives new mesh uploads

After uploading, buffers are atomically swapped. This prevents torn geometry from in-progress uploads.

### Fix 2: Keep READY State During Remeshing (Main Fix)

For chunks that are being **remeshed** (block modification), we now keep the state as `READY` so the chunk continues rendering with its old mesh while the new mesh is being built.

The flow is now:

**First-time meshing** (new chunk):
```
GENERATING → GENERATED → MESHING → MESHED → READY
```

**Remeshing** (block modification):
```
READY (dirty=true) → READY (meshing in background) → READY (new mesh swapped in)
```

## Changes Made

### World.cpp

**setBlock()**: No longer changes state to `GENERATED`. Just marks the chunk dirty:

```cpp
chunk->setBlock(x, y, z, id);
chunk->setDirty(true);
// Keep state as READY so the chunk continues rendering
```

**update()**: Swaps block buffers for any chunk with pending changes (not just GENERATED):

```cpp
if (chunk.hasPendingChanges())
    chunk.swapBuffers();
```

### ChunkMeshManager.cpp

**scheduleMeshing()**: Handles both first-time meshing and remeshing:

```cpp
const bool needsFirstMesh = chunk.getState() == ChunkState::GENERATED;
const bool needsRemesh = chunk.getState() == ChunkState::READY && chunk.isDirty();

// For first-time meshing, change state to MESHING
// For remeshing, keep state as READY so chunk continues rendering
if (needsFirstMesh)
    chunk.setState(ChunkState::MESHING);
```

**buildMeshJob()**: Only changes state for first-time meshing:

```cpp
if (chunk->getState() == ChunkState::MESHING)
    chunk->setState(ChunkState::MESHED);
```

**update()**: Only changes state for first-time meshing:

```cpp
if (c->getState() == ChunkState::MESHED)
    c->setState(ChunkState::READY);
```

### ChunkManager.cpp

**rebuildNeighbors()**: For READY neighbors, marks dirty instead of changing state:

```cpp
if (n->getState() == ChunkState::READY)
    n->setDirty(true);
else if (n->getState() >= ChunkState::GENERATED)
    n->setState(ChunkState::GENERATED);
```

### ChunkMesh.h/cpp

Added double-buffered VAO/VBO with atomic swap:

```cpp
VAO buffers[2];
size_t vertexCounts[2]{0, 0};
std::atomic<uint8_t> frontBufferIndex{0};

void upload(MeshData&& data);   // Writes to back buffer
void swapBuffers();             // Atomically swap front/back
void render() const;            // Renders from front buffer
```

## Result

The chunk now continues rendering its old mesh while the new mesh is being built in the background. When the new mesh is ready, the VAO/VBO buffers are swapped atomically, and the new mesh appears instantly without any blink.

## Files Modified

| File | Changes |
|------|---------|
| `src/Content/World/World.cpp` | Keep READY state in setBlock(), always swap block buffers |
| `src/Content/ChunkMeshManager/ChunkMeshManager.cpp` | Handle remeshing without state change, double-buffer swap |
| `src/Content/ChunkMeshManager/ChunkMeshManager.h` | No changes needed |
| `src/Content/ChunkManager/ChunkManager.cpp` | Mark READY neighbors dirty instead of GENERATED |
| `src/Content/ChunkMesh/ChunkMesh.h` | Double-buffered VAO/VBO arrays, atomic index |
| `src/Content/ChunkMesh/ChunkMesh.cpp` | Implemented double-buffer upload/swap/render |
