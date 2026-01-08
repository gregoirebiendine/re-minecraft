
#ifndef RE_MINECRAFT_CUBE_VAO_H
#define RE_MINECRAFT_CUBE_VAO_H

#include <map>

#include "glad/glad.h"
#include "VBO.h"

class VAO {
    GLuint ID = -1;
    std::map<int, VBO> VBOs;

    public:
        VAO();
        ~VAO();

        template<typename T, int K>
        void addData(const std::vector<T> &vertices, int index, int size);

        void bind() const;
        void unbind() const;
};


#endif //RE_MINECRAFT_CUBE_VAO_H
