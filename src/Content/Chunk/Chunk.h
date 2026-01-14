#ifndef RE_MINECRAFT_CHUNK_H
#define RE_MINECRAFT_CHUNK_H

#define GLM_ENABLE_EXPERIMENTAL

#include <array>
#include <algorithm>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "BlockRegistry.h"
#include "ChunkPos.h"
#include "ChunkState.h"

class Chunk {
    public:
        static constexpr uint8_t SIZE = 16;
        static constexpr uint16_t VOLUME = SIZE * SIZE * SIZE;

        explicit Chunk(ChunkPos pos);

        [[nodiscard]] glm::mat<4, 4, float> getChunkModel() const;
        [[nodiscard]] ChunkPos getPosition() const;
        [[nodiscard]] const ChunkState& getState() const;
        void setState(ChunkState state);

        [[nodiscard]] bool isDirty() const;
        void setDirty(bool dirty);

        [[nodiscard]] bool isAir(uint8_t x, uint8_t y, uint8_t z) const;
        [[nodiscard]] Material getBlock(uint8_t x, uint8_t y, uint8_t z) const;
        void setBlock(uint8_t x, uint8_t y, uint8_t z, Material id);
        void fill(glm::ivec3 from, glm::ivec3 to, Material id);

    private:
        std::array<Material, VOLUME> _blocks{};
        ChunkPos _position{};
        bool _isDirty = true;
        ChunkState _state = ChunkState::EMPTY;

        // Statics
        [[nodiscard]] static uint16_t index(uint8_t x, uint8_t y, uint8_t z);
        [[nodiscard]] static uint8_t clamp(uint8_t v);
};

#endif //RE_MINECRAFT_CHUNK_H
