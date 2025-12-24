#ifndef RE_MINECRAFT_CHUNK_H
#define RE_MINECRAFT_CHUNK_H

#include <iostream>
#include <cmath>
#include <array>
#include <iostream>
#include <algorithm>
#include <glm/glm.hpp>
#include <map>

#include "CubeVAO.h"
#include "VAO.h"

enum BlockID : uint8_t;

class Chunk {
    static constexpr uint8_t SIZE = 16;
    static constexpr uint16_t VOLUME = SIZE * SIZE * SIZE;

    std::array<BlockID, VOLUME> blocks{};
    glm::uvec3 position{};

    CubeVAO VAO;
    std::vector<GLuint> vertices;
    std::vector<GLfloat> tex;

    public:
        explicit Chunk(glm::uvec3 offset);

        [[nodiscard]] static inline uint16_t index(uint8_t x, uint8_t y, uint8_t z);
        [[nodiscard]] static inline uint8_t clamp(uint8_t v);

        // Getters
        [[nodiscard]] BlockID getBlock(uint8_t x, uint8_t y, uint8_t z) const;
        [[nodiscard]] bool isAir(uint8_t x, uint8_t y, uint8_t z) const;

        // Setters
        void setBlock(uint8_t x, uint8_t y, uint8_t z, BlockID id);

        void bind() const;
        void render() const;
};

#endif //RE_MINECRAFT_CHUNK_H
