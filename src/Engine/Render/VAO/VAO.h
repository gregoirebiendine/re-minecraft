
#ifndef RE_MINECRAFT_CUBE_VAO_H
#define RE_MINECRAFT_CUBE_VAO_H

#include <map>
#include <vector>

#include "glad/glad.h"

#include "VBO.h"
#include "Vertex.h"

class VAO {
    GLuint ID{};
    std::map<int, VBO> VBOs;

    VBO BlockVBO;

    public:
        VAO();
        ~VAO();

        template<typename T, int K>
        void addData(const std::vector<T> &vertices, int index, int size);

        void storeBlockData(const std::vector<Vertex> &data) const;

        void bind() const;
        void unbind() const;
};


#endif //RE_MINECRAFT_CUBE_VAO_H
