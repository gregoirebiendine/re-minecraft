#include "World.h"

World::World() :
    chunk({0, 16, 0})
{

}


void World::render() const
{
    chunk.render();
}
