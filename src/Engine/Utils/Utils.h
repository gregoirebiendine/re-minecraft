#ifndef FARFIELD_UTILS_H
#define FARFIELD_UTILS_H

#include <random>
#include <filesystem>

#include <glm/glm.hpp>

#include "ChunkPos.h"

namespace fs = std::filesystem;

namespace glm {
    inline std::string operator+(const std::string &lhs, const vec3& v)
    {
        return lhs + std::to_string(static_cast<int>(v.x)) + ", " + std::to_string(static_cast<int>(v.y)) + ", " + std::to_string(static_cast<int>(v.z));
    }

    inline std::ostream& operator<<(std::ostream& os, const vec3& v)
    {
        return os << v.x << ", " << v.y << ", " << v.z;
    }
}

namespace Maths
{
    inline int randomInt(const int min, const int max)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(min, max);

        return distrib(gen);
    }

    template <typename T = float>
    T mapRange(T v, T minIn, T maxIn, T minOut, T maxOut)
    {
        return minOut + (maxOut - minOut) * ((std::clamp(v, minIn, maxIn) - minIn) / (maxIn - minIn));
    }
}

namespace Files
{
    static std::string getResourcesPath(const std::string& path)
    {
        return RESOURCES_PATH + path;
    }
}

#endif