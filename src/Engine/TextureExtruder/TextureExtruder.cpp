#include "TextureExtruder.h"

#include <glm/gtc/matrix_inverse.hpp>

std::vector<EntityVertex> TextureExtruder::generate(const uint8_t* pixels, const int w, const int h, const glm::mat4& transform)
{
    constexpr float z0 = DEPTH/2.f;     // front
    constexpr float z1 = -z0;           // back

    std::vector<EntityVertex> vertices;
    vertices.reserve(w * h * 6);

    const float pw = 1.0f / static_cast<float>(w);
    const float ph = 1.0f / static_cast<float>(h);

    for (int v = 0; v < h; v++) {
        for (int u = 0; u < w; u++) {

            if (!isSolid(pixels, w, h, u, v))
                continue;

            const float x0 = static_cast<float>(u) * pw - 0.5f;
            const float x1 = static_cast<float>(u + 1) * pw - 0.5f;

            const float y0 = static_cast<float>(h - v - 1) * ph - 0.5f;
            const float y1 = static_cast<float>(h - v) * ph - 0.5f;

            const float tu0 = static_cast<float>(u) * pw;
            const float tu1 = static_cast<float>(u + 1) * pw;
            const float tv0 = static_cast<float>(v) * ph;
            const float tv1 = static_cast<float>(v + 1) * ph;

            emitQuad(vertices,
                {x0, y0, z0}, {x1, y0, z0}, {x1, y1, z0}, {x0, y1, z0},
                {tu0, tv1},   {tu1, tv1},   {tu1, tv0},   {tu0, tv0},
                {0.0f, 0.0f, 1.0f});

            emitQuad(vertices,
                {x1, y0, z1}, {x0, y0, z1}, {x0, y1, z1}, {x1, y1, z1},
                {tu1, tv1},   {tu0, tv1},   {tu0, tv0},   {tu1, tv0},
                {0.0f, 0.0f, -1.0f});

            if (!isSolid(pixels, w, h, u - 1, v)) {
                emitQuad(vertices,
                    {x0, y0, z1}, {x0, y0, z0}, {x0, y1, z0}, {x0, y1, z1},
                    {tu0, tv1},   {tu0, tv1},   {tu0, tv0},   {tu0, tv0},
                    {-1.0f, 0.0f, 0.0f});
            }

            if (!isSolid(pixels, w, h, u + 1, v)) {
                emitQuad(vertices,
                    {x1, y0, z0}, {x1, y0, z1}, {x1, y1, z1}, {x1, y1, z0},
                    {tu1, tv1},   {tu1, tv1},   {tu1, tv0},   {tu1, tv0},
                    {1.0f, 0.0f, 0.0f});
            }

            if (!isSolid(pixels, w, h, u, v - 1)) {
                emitQuad(vertices,
                    {x0, y1, z0}, {x1, y1, z0}, {x1, y1, z1}, {x0, y1, z1},
                    {tu0, tv0},   {tu1, tv0},   {tu1, tv0},   {tu0, tv0},
                    {0.0f, 1.0f, 0.0f});
            }

            if (!isSolid(pixels, w, h, u, v + 1)) {
                emitQuad(vertices,
                    {x1, y0, z0}, {x0, y0, z0}, {x0, y0, z1}, {x1, y0, z1},
                    {tu1, tv1},   {tu0, tv1},   {tu0, tv1},   {tu1, tv1},
                    {0.0f, -1.0f, 0.0f});
            }
        }
    }

    if (transform != glm::mat4(1.0f)) {
        const glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(transform));
        for (auto& vtx : vertices) {
            vtx.position = glm::vec3(transform * glm::vec4(vtx.position, 1.0f));
            vtx.normal   = glm::normalize(normalMatrix * vtx.normal);
        }
    }

    return vertices;
}

bool TextureExtruder::isSolid(const uint8_t* pixels, const int w, const int h, const int u, const int v)
{
    if (u < 0 || u >= w || v < 0 || v >= h)
        return false;
    return pixels[(v * w + u) * 4 + 3] >= ALPHA_THRESHOLD;
}

void TextureExtruder::emitQuad(
    std::vector<EntityVertex>& out,
    glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3,
    glm::vec2 uv0, glm::vec2 uv1, glm::vec2 uv2, glm::vec2 uv3,
    glm::vec3 normal)
{
    out.push_back({v0, normal, uv0});
    out.push_back({v1, normal, uv1});
    out.push_back({v2, normal, uv2});

    out.push_back({v0, normal, uv0});
    out.push_back({v2, normal, uv2});
    out.push_back({v3, normal, uv3});
}
