#include "Chunk.h"

constexpr uint16_t MAX_BLOCK = 4096;

Chunk::Chunk() :
    data(MAX_BLOCK, 1)
{
    // Get 3D coords from 1D index
    // const int index = 2047;
    // const int size = 16;
    // int x = index / (size * size);
    // int y = (index / size) % size;
    // int z = index % size;

    const int texOffset = 1;
    const glm::vec2 computedTexOffset = glm::vec2(0.25f * (texOffset % 4), floor(texOffset / 4) / 4);
    const glm::vec2 computedTexOffset2 = glm::vec2(0.25f * (2 % 4), floor(2 / 4) / 4);

    // this->vertices = {
    //     -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,         // Front face
    //     -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f,     // Left face
    //     1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f,     // Back face
    //     1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f,         // Right face
    //     -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,         // Top face
    //     -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f      // Bottom face
    // };

    this->vertices = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,         // Front face

        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, -1.0f,     // Left face

        1.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,     // Back face

        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 0.0f,         // Right face

        0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, -1.0f,
        0.0f, 1.0f, -1.0f,         // Top face

        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, -1.0f,
        1.0f, 0.0f, -1.0f,
        1.0f, 0.0f, 0.0f      // Bottom face
    };

    // Indices v1
    this->indices = {
        0, 1, 2, 0, 2, 3,
        4, 5, 6, 4, 6, 7,
        8, 9, 10, 8, 10, 11,
        12, 13, 14, 12, 14, 15,
        16, 17, 18, 16, 18, 19,
        20, 21, 22, 20, 22, 23,
        // i, i+1, i+2, i, i+2, i+3, ((i+=4))
    };

    this->tex = {
        0.0f, 0.0f, 0.25f, 0.0f, 0.25f, 0.25f, 0.0f, 0.25f,
        0.0f + computedTexOffset.x, 0.0f + computedTexOffset.y, 0.25f + computedTexOffset.x, 0.0f + computedTexOffset.y, 0.25f + computedTexOffset.x, 0.25f + computedTexOffset.y, 0.0f + computedTexOffset.x, 0.25f + computedTexOffset.y,
        0.0f + computedTexOffset2.x, 0.0f + computedTexOffset2.y, 0.25f + computedTexOffset2.x, 0.0f + computedTexOffset2.y, 0.25f + computedTexOffset2.x, 0.25f + computedTexOffset2.y, 0.0f + computedTexOffset2.x, 0.25f + computedTexOffset2.y,
        0.0f, 0.0f, 0.25f, 0.0f, 0.25f, 0.25f, 0.0f, 0.25f,
        0.0f, 0.0f, 0.25f, 0.0f, 0.25f, 0.25f, 0.0f, 0.25f,
        0.0f, 0.0f, 0.25f, 0.0f, 0.25f, 0.25f, 0.0f, 0.25f,
    };

    // Declare base vertices and UVs of a face (OLD)
    // this->vertices = {
    //     0.0f, 0.0f, 0.0f,       0.0f, 0.0f,         // Bottom Left
    //     1.0f, 0.0f, 0.0f,       0.25f, 0.0f,        // Bottom Right
    //     1.0f, 1.0f, 0.0f,       0.25f, 0.25f,       // Top Right
    //     0.0f, 1.0f, 0.0f,       0.0f, 0.25f,        // Top Left
    // };

    // Declare indices needed to draw a square (face) with 2 triangles (OLD)
    // this->indices = {
    //     0, 1, 2, 0, 2, 3,
    // };

    // Declare UV offsets per face instance.
    // Is equal to atlas texture id (0: dirt / 1: grass_side / 2: grass_top / ...)
    // this->texOffsets = {
    //     1, 1, 1, 1, 2, 0, // Block 1
    //     0, 0, 0, 0, 0, 0, // Block 2
    //     2, 2, 2, 2, 2, 2, // Block 3
    // };

    // Bind VAO
    this->VAO.bind();

    // Link datas to VA0 before rendering
    this->VAO.linkVertices(this->vertices, this->indices);
    this->VAO.linkTex(this->tex);
    // this->VAO.linkTexOffset(this->texOffsets);
    // this->VAO.linkPosOffset(this->posOffsets);
    // this->VAO.linkRenderedSides(this->renderedSides);

    // Unbind VAO
    this->VAO.unbind();
}

void Chunk::bind() const {
    this->VAO.bind();
}

void Chunk::draw() const
{
    // Bind VAO
    this->bind();

    // Draw instances (faces)
//    glDrawElementsInstanced(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0, this->renderedSides.size());

    glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
}
