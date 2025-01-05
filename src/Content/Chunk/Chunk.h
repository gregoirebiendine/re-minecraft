#ifndef RE_MINECRAFT_CHUNK_H
#define RE_MINECRAFT_CHUNK_H

#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "Square.h"

class Chunk {
    VAO VAO1;
    VBO VBO1;
    EBO EBO1;

    std::vector<Square> squares;
    std::vector<GLfloat> verts;
    std::vector<GLuint> inds;

    public:
        Chunk();

        void bind() const;
        void draw() const;
        void registerSquares();
};

#endif //RE_MINECRAFT_CHUNK_H
