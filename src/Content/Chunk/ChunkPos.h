#ifndef RE_MINECRAFT_CHUNKPOS_H
#define RE_MINECRAFT_CHUNKPOS_H

#include <functional>
#include <iostream>

struct ChunkPos {
    int x, y, z;

    bool operator==(const ChunkPos& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

struct ChunkPosHash {
    size_t operator()(const ChunkPos& p) const {
        const size_t h1 = std::hash<int>()(p.x);
        const size_t h2 = std::hash<int>()(p.y);
        const size_t h3 = std::hash<int>()(p.z);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

inline std::ostream& operator<<(std::ostream& os, const ChunkPos pos)
{
    return os << "(" << pos.x << ' ' << pos.y << ' ' << pos.z << ")";
}

inline ChunkPos operator*(const ChunkPos pos, const int mul)
{
    return {pos.x * mul, pos.y * mul, pos.z * mul};
}

#endif //RE_MINECRAFT_CHUNKPOS_H