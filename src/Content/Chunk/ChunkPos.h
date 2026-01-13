#ifndef RE_MINECRAFT_CHUNKPOS_H
#define RE_MINECRAFT_CHUNKPOS_H

#include <functional>
#include <iostream>

#include <glm/glm.hpp>

struct BlockPos
{
    int x, y, z;

    bool operator==(const BlockPos& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

struct ChunkPos {
    int x, y, z;

    static ChunkPos fromWorld(const int wx, const int wy, const int wz)
    {
        return {wx >> 4, wy >> 4, wz >> 4};
    }

    static ChunkPos fromWorld(const glm::vec3& pos)
    {
        return {static_cast<int>(pos.x) >> 4, static_cast<int>(pos.y) >> 4, static_cast<int>(pos.z) >> 4};
    }

    bool operator<(const ChunkPos& other) const
    {
        return x < other.x && y < other.y && z < other.z;
    }

    bool operator==(const ChunkPos& other) const
    {
        return x == other.x && y == other.y && z == other.z;
    }

    bool operator!=(const ChunkPos& other) const
    {
        return x != other.x || y != other.y || z != other.z;
    }
};

struct ChunkPosHash {
    std::size_t operator()(const ChunkPos& p) const noexcept {
        std::size_t h1 = std::hash<int>{}(p.x);
        std::size_t h2 = std::hash<int>{}(p.y);
        std::size_t h3 = std::hash<int>{}(p.z);
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