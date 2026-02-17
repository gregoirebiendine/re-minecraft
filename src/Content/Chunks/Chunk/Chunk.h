#ifndef FARFIELD_CHUNK_H
#define FARFIELD_CHUNK_H

#define GLM_ENABLE_EXPERIMENTAL

#pragma once

#include <iostream>
#include <algorithm>
#include <thread>
#include <atomic>
#include <array>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "BlockRegistry.h"
#include "ChunkPos.h"
#include "ChunkState.h"
#include "Utils.h"

using BlockStorage = std::array<Material, 16*16*16>;

class Chunk {
    public:
        static constexpr uint8_t SIZE = 16;
        static constexpr uint16_t VOLUME = SIZE * SIZE * SIZE;

        explicit Chunk(ChunkPos pos);
        Chunk(const Chunk&) = delete;
        Chunk& operator=(const Chunk&) = delete;
        Chunk(Chunk&& other) noexcept;
        Chunk& operator=(Chunk&& other) noexcept;

        uint8_t acquireRead() const;
        void releaseRead() const;

        [[nodiscard]] BlockStorage getBlockSnapshot() const;

        [[nodiscard]] Material getBlock(uint8_t x, uint8_t y, uint8_t z) const;
        [[nodiscard]] bool isAir(uint8_t x, uint8_t y, uint8_t z) const;

        void setBlock(uint8_t x, uint8_t y, uint8_t z, Material mat);
        void fill(glm::ivec3 from, glm::ivec3 to, Material mat);

        void setBlockDirect(uint8_t x, uint8_t y, uint8_t z, Material mat);
        void fillDirect(glm::ivec3 from, glm::ivec3 to, Material mat);

        bool swapBuffers();
        [[nodiscard]] bool hasPendingChanges() const;
        void finalizeGeneration();

        [[nodiscard]] glm::mat4 getChunkModel() const;
        [[nodiscard]] ChunkPos getPosition() const;

        [[nodiscard]] ChunkState getState() const;
        void setState(ChunkState newState);

        [[nodiscard]] bool isDirty() const;
        void setDirty(bool isDirty);

        [[nodiscard]] uint64_t getGenerationID() const;
        void bumpGenerationID();

    private:
        ChunkPos position;

        BlockStorage blockBuffers[2]{};
        std::atomic<uint8_t> bufferReadIndex{0};
        mutable std::atomic<uint32_t> bufferActiveReaders{0};
        std::atomic<bool> pendingChanges{false};

        // Chunk lifecycle state
        std::atomic<ChunkState> state{ChunkState::UNLOADED};
        std::atomic<uint64_t> generationID{0};
        std::atomic<bool> dirty{false};

        [[nodiscard]] uint8_t getWriteIndex() const
        {
            return 1 - this->bufferReadIndex.load(std::memory_order_acquire);
        }
};

#endif
