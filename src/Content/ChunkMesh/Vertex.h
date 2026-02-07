#ifndef RE_MINECRAFT_VERTEX_H
#define RE_MINECRAFT_VERTEX_H

#include <glm/glm.hpp>

struct PackedBlockVertex {
    uint32_t data1;  // position + normal index + rotation + ambient occlusion
    uint32_t data2;  // uv + texId

    PackedBlockVertex() : data1(0), data2(0) {}

    PackedBlockVertex(
        const uint8_t x, const uint8_t y, const uint8_t z,
        const uint8_t normalIndex, const uint8_t rotation,
        const uint8_t u, const uint8_t v,
        const uint16_t texId, const uint8_t ao = 0
    )
    {
        data1 = (x & 0x1F)
              | ((y & 0x1F) << 5)
              | ((z & 0x1F) << 10)
              | ((normalIndex & 0x7) << 15)
              | ((rotation & 0x7) << 18)
              | ((ao & 0xF) << 28);

        data2 = (u & 0x1F)
              | ((v & 0x1F) << 5)
              | ((static_cast<uint32_t>(texId) & 0xFFFF) << 10);
    }

    static PackedBlockVertex create(
        const uint8_t x, const uint8_t y, const uint8_t z, const uint8_t normalIndex,
        const uint8_t u, const uint8_t v, const uint16_t texId
    )
    {
        return {x, y, z, normalIndex, 0, u, v, texId, 0};
    }
};

struct GuiVertex {
    glm::vec2 position;
    glm::vec4 color;
};

struct EntityVertex {
    glm::vec3 position;
    // glm::vec3 normal;
    // glm::vec2 uv;
};


#endif //RE_MINECRAFT_VERTEX_H