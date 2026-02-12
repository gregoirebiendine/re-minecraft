#ifndef FARFIELD_UTILS_H
#define FARFIELD_UTILS_H

#include <random>
#include <filesystem>
#include "ChunkPos.h"

namespace fs = std::filesystem;

namespace ChunkCoords
{
    constexpr uint8_t ChunkSize = 16;
    
    // Convert a 1D index to 3D coordinates (x, y, z)
    inline BlockPos indexToLocalCoords(const int index)
    {
        return {
            index % ChunkSize,
            (index / ChunkSize) % ChunkSize,
            index / (ChunkSize * ChunkSize)
        };
    }

    // Convert a local 3D coordinates to a 1D index
    inline int localCoordsToIndex(const uint8_t lx, const uint8_t ly, const uint8_t lz)
    {
        return lx + ChunkSize * (ly + ChunkSize * lz);
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
    static const fs::path SOURCE_FOLDER =  fs::current_path().parent_path();
}

#endif