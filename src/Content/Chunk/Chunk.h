#ifndef RE_MINECRAFT_CHUNK_H
#define RE_MINECRAFT_CHUNK_H

#define GLM_ENABLE_EXPERIMENTAL

#include <atomic>
#include <array>
#include <algorithm>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "BlockRegistry.h"
#include "ChunkPos.h"
#include "ChunkState.h"
#include "Utils.h"

class Chunk {
    public:
        static constexpr uint8_t SIZE = 16;
        static constexpr uint16_t VOLUME = SIZE * SIZE * SIZE;

        explicit Chunk(ChunkPos pos);

        [[nodiscard]] glm::mat<4, 4, float> getChunkModel() const;
        [[nodiscard]] ChunkPos getPosition() const;

        [[nodiscard]] bool isAir(uint8_t x, uint8_t y, uint8_t z) const;
        [[nodiscard]] Material getBlock(uint8_t x, uint8_t y, uint8_t z) const;
        void setBlock(uint8_t x, uint8_t y, uint8_t z, Material id);
        void fill(glm::ivec3 from, glm::ivec3 to, Material id);

        [[nodiscard]] ChunkState getState() const;
        void setState(ChunkState _state);

        [[nodiscard]] uint64_t getGenerationID() const;
        void bumpGenerationID();

    private:
        std::array<Material, VOLUME> blocks{};
        ChunkPos position;

        std::atomic<ChunkState> state{ChunkState::UNLOADED};
        std::atomic<uint64_t> generationID{0};
};

#endif //RE_MINECRAFT_CHUNK_H
