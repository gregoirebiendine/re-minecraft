#ifndef FARFIELD_NEIGHBORACCESS_H
#define FARFIELD_NEIGHBORACCESS_H

#include <array>

#include "Chunk.h"
#include "Material.h"

class NeighborAccess {
    public:
        NeighborAccess(const ChunkPos& _centerPos, std::function<Chunk*(const ChunkPos&)> getChunk);

        Chunk* getCenter() const { return this->chunks[13]; }

        Material getBlock(int wx, int wy, int wz) const;
        void setBlock(int wx, int wy, int wz, Material mat);

        bool allNeighborsReady() const;
        void markDirtyChunks() const;

    private:
        ChunkPos centerPos;
        std::array<Chunk*, 27> chunks;
        std::array<bool, 27> chunkModified;

        // Convert 3D offset (-1,0,1) to array index (0-26)
        static int offsetToIndex(int dx, int dy, int dz) {
            return (dx + 1) + (dy + 1) * 3 + (dz + 1) * 9;
        }

        // Get chunk containing world position
        Chunk* getChunkForWorldPos(int wx, int wy, int wz) const;
};


#endif