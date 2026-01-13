#ifndef RE_MINECRAFT_MATERIAL_H
#define RE_MINECRAFT_MATERIAL_H

using Material = unsigned short;

enum MaterialFace : char
{
    NORTH = 0,
    SOUTH,
    WEST,
    EAST,
    UP,
    DOWN
};

#endif //RE_MINECRAFT_MATERIAL_H