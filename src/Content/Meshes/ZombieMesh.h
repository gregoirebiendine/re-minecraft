#ifndef RE_MINECRAFT_ZOMBIEMESH_H
#define RE_MINECRAFT_ZOMBIEMESH_H

#include <vector>
#include "glad/glad.h"
#include "glm/vec3.hpp"
#include "Vertex.h"

static const std::vector<EntityVertex> ZOMBIE_MESH = {
    {{-0.3f,  1.529184,    0.3f}},
    {{-0.3f,  2.529183,    0.3f}},
    {{-0.3f,  1.529184,    -0.3f}},
    {{-0.3f,  2.529183,    -0.3f}},
    {{0.3f,   1.529184,     0.3f}},
    {{0.3f,   2.529183,     0.3f}},
    {{0.3f,   1.529184,     -0.3f}},
    {{0.3f,   2.529183,     -0.3f}},
    {{-0.8f,  -0.196642,   0.8f}},
    {{-0.8f,  1.403358,    0.8f}},
    {{-0.8f,  -0.196642,   -0.8f}},
    {{-0.8f,  1.403358,    -0.8f}},
    {{0.8f,   -0.196642,    0.8f}},
    {{0.8f,   1.403358,     0.8f}},
    {{0.8f,   -0.196642,    -0.8f}},
    {{0.8f,   1.403358,     -0.8f}}
};

#endif