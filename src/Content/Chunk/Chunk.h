#ifndef RE_MINECRAFT_CHUNK_H
#define RE_MINECRAFT_CHUNK_H

#define GLM_ENABLE_EXPERIMENTAL

#include <array>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <Materials.h>

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

class Chunk {
    public:
        static constexpr uint8_t SIZE = 16;
        static constexpr uint16_t VOLUME = SIZE * SIZE * SIZE;

        explicit Chunk(ChunkPos pos);

        // Getters
        [[nodiscard]] glm::mat<4, 4, float> getChunkModel() const;
        [[nodiscard]] ChunkPos getPosition() const;
        [[nodiscard]] Material getBlock(uint8_t x, uint8_t y, uint8_t z) const;
        [[nodiscard]] bool isAir(uint8_t x, uint8_t y, uint8_t z) const;
        [[nodiscard]] bool isDirty() const;

        // Setters
        void setBlock(uint8_t x, uint8_t y, uint8_t z, Material id);
        void setDirty(bool dirty);

    private:
        std::array<Material, VOLUME> _blocks{};
        ChunkPos _position{};
        bool _isDirty = true;

        [[nodiscard]] static uint16_t index(uint8_t x, uint8_t y, uint8_t z);
        [[nodiscard]] static uint8_t clamp(uint8_t v);
};

#endif //RE_MINECRAFT_CHUNK_H
