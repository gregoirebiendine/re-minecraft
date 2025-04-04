#include "Chunk.h"

constexpr uint16_t MAX_BLOCK = 4096;

Chunk::Chunk() :
    data(MAX_BLOCK, 1)
{
    // const int index = 2047;
    // const int size = 16;
    // int x = index / (size * size);
    // int y = (index / size) % size;
    // int z = index % size;


    std::vector<GLint> renderedSidesPerBlock;
    std::vector<GLuint> texOffsetsPerBlock;

    for (int i = 0; i < MAX_BLOCK; i++) {
        if (i == 0) {
            renderedSidesPerBlock = {0, 1, 2, 4, 5};
            texOffsetsPerBlock = {1, 1, 1, 2, 0};
        }
        else if (i == MAX_BLOCK - 1) {
            renderedSidesPerBlock = {0, 1, 3, 4, 5};
            texOffsetsPerBlock = {1, 1, 1, 2, 0};
        }
        else {
            renderedSidesPerBlock = {0, 1, 4, 5};
            texOffsetsPerBlock = {1, 1, 2, 0};
        }

        this->renderedSides.insert(this->renderedSides.end(), renderedSidesPerBlock.begin(), renderedSidesPerBlock.end());
        this->texOffsets.insert(this->texOffsets.end(), texOffsetsPerBlock.begin(), texOffsetsPerBlock.end());
        for (int j = 0; j < renderedSidesPerBlock.size(); j++) {
            this->posOffsets.insert(this->posOffsets.end(), {static_cast<float>(i), 0.0f, 0.0f});
        }
    }

    // for (int i = 0; i < 16; i++) {
    //     for (int j = 0; j < 16; j++) {
    //         this->texOffsets.insert(this->texOffsets.end(), {1, 1, 1, 1, 2, 0});
    //         this->posOffsets.insert(this->posOffsets.end(), {j, 0.0f, i});
    //
    //         if (i == 0)
    //             this->renderedSides.insert(this->renderedSides.end(), {0, 1, 2, -1, 4, 5});
    //         else if (i == 15)
    //             this->renderedSides.insert(this->renderedSides.end(), {0, 1, -1, 3, 4, 5});
    //         else
    //             this->renderedSides.insert(this->renderedSides.end(), {0, 1, -1, -1, 4, 5});
    //     }
    // }

    // Declare base vertices and UVs of a face
    this->vertices = {
        0.0f, 0.0f, 0.0f,       0.0f, 0.0f,         // Bottom Left
        1.0f, 0.0f, 0.0f,       0.25f, 0.0f,        // Bottom Right
        1.0f, 1.0f, 0.0f,       0.25f, 0.25f,       // Top Right
        0.0f, 1.0f, 0.0f,       0.0f, 0.25f,        // Top Left
    };

    // Declare indices needed to draw a square (face) with 2 triangles
    this->indices = {
        0, 1, 2, 0, 2, 3,
    };

    // Declare UV offsets per face instance.
    // Is equal to atlas texture id (0: dirt / 1: grass_side / 2: grass_top / ...)
    // this->texOffsets = {
    //     1, 1, 1, 1, 2, 0, // Block 1
    //     0, 0, 0, 0, 0, 0, // Block 2
    //     2, 2, 2, 2, 2, 2, // Block 3
    // };

    // Declare position offset of each cubes
    // this->posOffsets = {
    //     0.0f, 0.0f, 0.0f,
    //     0.0f, 0.0f, 0.0f,
    //     0.0f, 0.0f, 0.0f,
    //     0.0f, 0.0f, 0.0f,
    //     0.0f, 0.0f, 0.0f,
    //     0.0f, 0.0f, 0.0f,
    //     1.0f, 0.0f, 0.0f,
    //     1.0f, 0.0f, 0.0f,
    //     1.0f, 0.0f, 0.0f,
    //     1.0f, 0.0f, 0.0f,
    //     1.0f, 0.0f, 0.0f,
    //     1.0f, 0.0f, 0.0f,
    //     2.0f, 0.0f, 0.0f,
    //     2.0f, 0.0f, 0.0f,
    //     2.0f, 0.0f, 0.0f,
    //     2.0f, 0.0f, 0.0f,
    //     2.0f, 0.0f, 0.0f,
    //     2.0f, 0.0f, 0.0f,
    // };

    // Declare rendered faces of each cubes (-1 to not render)
    // Front, Back, Left, Right, Top, Bottom
    // this->renderedSides = {
    //     0, 1, 2, -1, 4, 5,
    //     0, 1, -1, -1, 4, 5,
    //     0, 1, -1, 3, 4, 5,
    // };

    // Bind VAO
    this->VAO.bind();

    // Link datas to VA0 before rendering
    this->VAO.linkVertices(this->vertices, this->indices);
    this->VAO.linkTexOffset(this->texOffsets);
    this->VAO.linkPosOffset(this->posOffsets);
    this->VAO.linkRenderedSides(this->renderedSides);

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
    glDrawElementsInstanced(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0, this->renderedSides.size());
}
