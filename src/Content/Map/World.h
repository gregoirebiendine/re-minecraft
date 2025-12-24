#ifndef WORLD_H
#define WORLD_H

#include <array>
#include <Chunk.h>

class World {
    Chunk chunk;

    public:
        World();

        void render() const;
};

#endif //WORLD_H
