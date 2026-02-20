
#ifndef FARFIELD_VAO_H
#define FARFIELD_VAO_H

#include <vector>

#include "glad/glad.h"

#include "VBO.h"
#include "VAOVertices.h"

class VAO {
    GLuint ID{};
    VBO vbo;

    public:
        VAO();
        ~VAO();

        void storeBlockData(const std::vector<PackedBlockVertex> &data) const;
        void storeGuiData(const std::vector<GuiVertex> &data) const;
        void storeEntityMeshData(const std::vector<EntityVertex> &data) const;
        void storeOutlineData(const std::vector<GLfloat> &data) const;
        void storeMsdfData(const std::vector<MSDFVertex>& data) const;

        void bind() const;
        void unbind() const;
};


#endif
