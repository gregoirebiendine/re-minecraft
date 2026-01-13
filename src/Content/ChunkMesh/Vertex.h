#ifndef RE_MINECRAFT_VERTEX_H
#define RE_MINECRAFT_VERTEX_H

#include <glm/glm.hpp>

struct Vertex {
    glm::ivec3 position;
    glm::ivec3 normal;
    glm::ivec2 uv;
    unsigned int texId;
};

#endif //RE_MINECRAFT_VERTEX_H