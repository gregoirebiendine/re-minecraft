#ifndef RE_MINECRAFT_MATERIALS_H
#define RE_MINECRAFT_MATERIALS_H

#include <glm/glm.hpp>
#include <array>
#include <map>

enum Material : uint8_t
{
    AIR,
    DIRT,
    GRASS,
    MOSS,
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

inline std::map<Material, std::array<uint8_t, 6>> MaterialTexFaces = {
    {Material::AIR, {0, 0, 0, 0, 0, 0}},
    {Material::DIRT, {0, 0, 0, 0, 0, 0}},
    {Material::GRASS, {1, 1, 1, 1, 2, 0}},
    {Material::MOSS, {2, 2, 2, 2, 2, 2}},
};

#endif //RE_MINECRAFT_MATERIALS_H