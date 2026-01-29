#include "NeighborAccess.h"

NeighborAccess::NeighborAccess(const ChunkPos& _centerPos, std::function<Chunk*(const ChunkPos&)> getChunk) :
    centerPos(_centerPos)
{
    this->chunkModified.fill(false);
    
    for (int dy = -1; dy <= 1; dy++) {
        for (int dz = -1; dz <= 1; dz++) {
            for (int dx = -1; dx <= 1; dx++) {
                ChunkPos neighborPos = {
                    _centerPos.x + dx,
                    _centerPos.y + dy,
                    _centerPos.z + dz
                };
                const int index = offsetToIndex(dx, dy, dz);
                this->chunks[index] = getChunk(neighborPos);
            }
        }
    }
}

bool NeighborAccess::allNeighborsReady() const
{
    // Check all 27 positions in the 3x3x3 neighborhood
    for (int dy = -1; dy <= 1; dy++) {
        for (int dz = -1; dz <= 1; dz++) {
            for (int dx = -1; dx <= 1; dx++) {
                const int index = offsetToIndex(dx, dy, dz);
                const Chunk* chunk = this->chunks[index];

                const int neighborY = this->centerPos.y + dy;

                if (chunk == nullptr) {
                    if (neighborY >= 0)
                        return false;
                    continue;
                }

                if (!hasTerrainComplete(chunk->getState()))
                    return false;
            }
        }
    }
    return true;
}

Material NeighborAccess::getBlock(const int wx, const int wy, const int wz) const
{
    const Chunk* chunk = getChunkForWorldPos(wx, wy, wz);

    if (!chunk)
        return 0;

    const auto [x, y, z] = BlockPos::fromWorld(wx, wy, wz);
    return chunk->getBlock(x, y, z);
}

void NeighborAccess::setBlock(const int wx, const int wy, const int wz, const Material mat)
{
    const auto [cx, cy, cz] = ChunkPos::fromWorld(wx, wy, wz);

    const int dx = cx - this->centerPos.x;
    const int dy = cy - this->centerPos.y;
    const int dz = cz - this->centerPos.z;

    // Check if within 3x3x3 neighborhood - silently ignore blocks outside
    if (dx < -1 || dx > 1 || dy < -1 || dy > 1 || dz < -1 || dz > 1) {
        return;
    }

    const int index = offsetToIndex(dx, dy, dz);
    Chunk* chunk = this->chunks[index];

    if (chunk && hasTerrainComplete(chunk->getState())) {
        const auto [x, y, z] = BlockPos::fromWorld(wx, wy, wz);

        chunk->setBlockDirect(x, y, z, mat);
        this->chunkModified[index] = true;
    }
}

Chunk* NeighborAccess::getChunkForWorldPos(const int wx, const int wy, const int wz) const
{
    const auto [cx, cy, cz] = ChunkPos::fromWorld(wx, wy, wz);

    const int dx = cx - this->centerPos.x;
    const int dy = cy - this->centerPos.y;
    const int dz = cz - this->centerPos.z;

    if (dx < -1 || dx > 1 || dy < -1 || dy > 1 || dz < -1 || dz > 1)
        return nullptr;

    return this->chunks[offsetToIndex(dx, dy, dz)];
}

void NeighborAccess::markDirtyChunks() const
{
    for (int i = 0; i < 27; i++) {
        if (this->chunkModified[i] && this->chunks[i])
            this->chunks[i]->setDirty(true);
    }
}
