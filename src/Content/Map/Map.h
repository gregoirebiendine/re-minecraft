#ifndef MAP_H
#define MAP_H

#include <array>
#include <Chunk.h>

class Map {
    // std::array<Chunk, 1> chunks;
    Chunk chunk;

    public:
        Map() = default;

        void draw() const;
};



#endif //MAP_H
