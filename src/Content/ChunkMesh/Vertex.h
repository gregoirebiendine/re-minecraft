#ifndef RE_MINECRAFT_VERTEX_H
#define RE_MINECRAFT_VERTEX_H

#include <glm/glm.hpp>

struct Vertex {
    glm::ivec3 position;
    glm::ivec3 normal;
    glm::ivec2 uv;
    unsigned int texId;
};

struct GuiVertex {
    glm::vec2 position;
    glm::vec4 color;
};


#endif //RE_MINECRAFT_VERTEX_H