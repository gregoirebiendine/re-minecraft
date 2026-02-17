#ifndef FARFIELD_UTILS_H
#define FARFIELD_UTILS_H

#include <random>
#include <filesystem>
#include "ChunkPos.h"

namespace fs = std::filesystem;

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
    static const fs::path SOURCE_FOLDER =  fs::current_path().parent_path();
}

#endif