
#include "Square.h"

Square::Square(std::vector<GLfloat> pos, std::vector<GLfloat> size)
{
    this->_pos = std::move(pos);
    this->_size = std::move(size);

    this->vertices = {
        _pos[0], _pos[1] + _size[1],                0.0f,       0.0f, 0.0f,     // Left lower corner
        _pos[0], _pos[1],                           0.0f,       0.0f, 1.0f,     // Left upper corner
        _pos[0] + _size[0], _pos[1],                0.0f,       1.0f, 1.0f,     // Right upper corner
        _pos[0] + _size[0], _pos[1] + _size[1],     0.0f,       1.0f, 0.0f,     // Right lower corner
    };
}

// Pushback vertices like this
// pos.x, pos.y + size.y,           0.0f        // Left lower corner
// pos.x, pos.y,                    0.0f        // Left upper corner
// pos.x + size.x, pos.y,           0.0f        // Right upper corner
// pos.x + size.x, pos.y + size.y,  0.0f        // Right lower corner

// Pushback indices like this for (j = 0; j+=4)
// 0, 2, 1, 0, 3, 2
// j, j+2, j+1, j, j+3, j+2

std::vector<GLfloat> &Square::getVertices() {
    return this->vertices;
}