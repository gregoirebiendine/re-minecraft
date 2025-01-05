
#ifndef RE_MINECRAFT_SQUARE_H
#define RE_MINECRAFT_SQUARE_H

#include <vector>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "VAO.h"
#include "VBO.h"

class Square {
        std::vector<GLfloat> _pos;
        std::vector<GLfloat> _size;
        std::vector<GLfloat> vertices;

    public:
        Square(std::vector<GLfloat> pos, std::vector<GLfloat> size);

        std::vector<GLfloat> &getVertices();
};


#endif //RE_MINECRAFT_SQUARE_H
