#ifndef FARFIELD_RGBA_H
#define FARFIELD_RGBA_H

#include "glm/glm.hpp"

struct RGBA
{
    float r;
    float g;
    float b;
    float a;

    RGBA(const uint8_t r, const uint8_t g, const uint8_t b, const float a)
    {
        this->r = static_cast<float>(r) / 255.f;
        this->g = static_cast<float>(g) / 255.f;
        this->b = static_cast<float>(b) / 255.f;
        this->a = a;
    }

    operator glm::vec4() const
    {
        return glm::vec4{this->r, this->g, this->b, this->a};
    }

    bool operator==(const RGBA& other) const
    {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }
};

#endif