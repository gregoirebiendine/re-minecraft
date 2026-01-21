
#ifndef RE_MINECRAFT_CUBE_VAO_H
#define RE_MINECRAFT_CUBE_VAO_H

#include <vector>

#include "glad/glad.h"

#include "VBO.h"
#include "Vertex.h"

class VAO {
    GLuint ID{};
    VBO BlockVBO;
    VBO GuiVBO;

    public:
        VAO();
        ~VAO();

        void storeBlockData(const std::vector<Vertex> &data) const;
        void storeGuiData(const std::vector<GuiVertex> &data) const;

        void bind() const;
        void unbind() const;
};


#endif //RE_MINECRAFT_CUBE_VAO_H
