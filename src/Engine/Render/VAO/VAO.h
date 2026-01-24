
#ifndef RE_MINECRAFT_CUBE_VAO_H
#define RE_MINECRAFT_CUBE_VAO_H

#include <vector>

#include "glad/glad.h"

#include "VBO.h"
#include "Vertex.h"

class VAO {
    GLuint ID{};
    VBO vbo;

    public:
        VAO();
        ~VAO();

        void storeBlockData(const std::vector<PackedBlockVertex> &data) const;
        void storeGuiData(const std::vector<GuiVertex> &data) const;
        void storeOutlineData(const std::vector<GLfloat> &data) const;

        void bind() const;
        void unbind() const;
};


#endif //RE_MINECRAFT_CUBE_VAO_H
