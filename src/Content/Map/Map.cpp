#include "Map.h"

Map::Map() :
    chunk({0, 16, 0}),
    chunk2({16, 16, 0})
{

}


void Map::draw() const
{
    // this->chunks[0].draw();
    chunk.draw();
    chunk2.draw();
}
