
#ifndef FARFIELD_VAO_H
#define FARFIELD_VAO_H

#include <vector>

#include "glad/glad.h"

#include "VBO.h"
#include "VAOVertices.h"

class VAO {
    VBO vbo;
    GLuint ID{};
    GLsizei size{};

    public:
        VAO();
        ~VAO();

        void storeBlockData(const std::vector<PackedBlockVertex> &data);
        void storeGuiData(const std::vector<GuiVertex> &data);
        void storeEntityMeshData(const std::vector<EntityVertex> &data);
        void storeOutlineData(const std::vector<GLfloat> &data);
        void storeMsdfData(const std::vector<MSDFVertex>& data);

        void bind() const;
        void unbind() const;
        void draw() const;
};


#endif
