#ifndef FARFIELD_TEXTUREEXTRUDER_H
#define FARFIELD_TEXTUREEXTRUDER_H

#pragma once

#include <vector>
#include <cstdint>

#include "VAOVertices.h"

class TextureExtruder {
    static bool isSolid(const uint8_t* pixels, int w, int h, int u, int v);

    static void emitQuad(
        std::vector<EntityVertex>& out,
        glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3,
        glm::vec2 uv0, glm::vec2 uv1, glm::vec2 uv2, glm::vec2 uv3,
        glm::vec3 normal
    );

    public:
        static constexpr uint8_t ALPHA_THRESHOLD = 1;
        static constexpr float DEPTH = 1.0f / 16.0f;

        static std::vector<EntityVertex> generate(const uint8_t* pixels, int w, int h, const glm::mat4& transform = glm::mat4(1.0f));
};

#endif