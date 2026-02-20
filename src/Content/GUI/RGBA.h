#ifndef FARFIELD_RGBA_H
#define FARFIELD_RGBA_H

#include "glm/glm.hpp"

struct RGBA
{
    float r;
    float g;
    float b;
    float a;

    RGBA(const float r, const float g, const float b, const float a) :
        r(r), g(g), b(b), a(a)
    {}

    static RGBA fromRGB(const uint8_t r, const uint8_t g, const uint8_t b, const float a)
    {
        return {
            static_cast<float>(r) / 255.f,
            static_cast<float>(g) / 255.f,
            static_cast<float>(b) / 255.f,
            a
        };
    }

    RGBA operator*(const float scalar) const
    {
        return {r * scalar, g * scalar, b * scalar, a};
    }

    bool operator==(const RGBA& other) const
    {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }

    operator glm::vec4() const
    {
        return glm::vec4{this->r, this->g, this->b, this->a};
    }
};

#endif