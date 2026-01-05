#ifndef RE_MINECRAFT_MATERIALS_H
#define RE_MINECRAFT_MATERIALS_H

#include <glm/glm.hpp>

#include <array>
#include <map>
#include <iostream>

using MaterialAtlasFaces = std::array<uint8_t, 6>;

enum Material : uint8_t
{
    AIR,
    DIRT,
    GRASS,
    MOSS,
    COBBLE,
    OAK_PLANK
};

enum MaterialFace : char
{
    FRONT = 0,
    BACK,
    LEFT,
    RIGHT,
    TOP,
    BOTTOM
};

inline std::map<Material, MaterialAtlasFaces> MaterialTexFaces = {
    {Material::AIR, {0, 0, 0, 0, 0, 0}},
    {Material::DIRT, {0, 0, 0, 0, 0, 0}},
    {Material::GRASS, {1, 1, 1, 1, 2, 0}},
    {Material::MOSS, {2, 2, 2, 2, 2, 2}},
    {Material::COBBLE, {3, 3, 3, 3, 3, 3}},
    {Material::OAK_PLANK, {4, 4, 4, 4, 4, 4}},
};

inline std::string MaterialToString(const Material material)
{
    switch(material)
    {
        default:
        case Material::AIR:    return "Air";
        case Material::DIRT:   return "Dirt";
        case Material::GRASS:  return "Grass";
        case Material::MOSS:   return "Moss";
        case Material::COBBLE:   return "Cobblestone";
        case Material::OAK_PLANK:   return "Oak Plank";
    }
}

inline std::ostream& operator<<(std::ostream& os, const Material block)
{
    return os << MaterialToString(block);
}

#endif //RE_MINECRAFT_MATERIALS_H