#include "Chunk.h"

std::vector<GLfloat> getUVFromIndex(const char index)
{
    const char tiles = 4;
    glm::vec2 UV(float(index%tiles)/tiles, 1.0f - ((std::round(index/tiles) + 1.0f) * (1.0f/tiles)));

    return {
            UV.x, UV.y,                                         // Left lower
            UV.x + (1.0f / tiles), UV.y,                        // Right lower
            UV.x + (1.0f / tiles), UV.y + (1.0f / tiles),       // Right upper
//            UV.x, UV.y,                                         // Left lower
//            UV.x + (1.0f / tiles), UV.y + (1.0f / tiles),       // Right upper
            UV.x, UV.y + (1.0f / tiles),                        // Left upper
    };
}

Chunk::Chunk()
{
//    Without Indices
//    this->vertices = {
//             // Front face
//            -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
//            -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
//
//             // Left face
//            -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
//            -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f,
//
//            // Right face
//            1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f,
//            1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
//
//            // Back face
//            1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
//            1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f,
//
//            // Top face
//            -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f,
//            -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
//
//            // Bottom face
//            -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
//            -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f
//    };

    // With Indices v1
    this->vertices = {
            // Front face
            -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,

            // Left face
            -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f,

            // Right face
            1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f,

            // Back face
            1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f,

            // Top face
            -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,

            // Bottom face
            -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f
    };

    this->indices = {
            0, 1, 2, 0, 2, 3,
            4, 5, 6, 4, 6, 7,
            8, 9, 10, 9, 10, 11,
            12, 13, 14, 12, 14, 15,
            16, 17, 18, 16, 18, 19,
            20, 21, 22, 21, 22, 23
    };

    std::vector<glm::vec3> faces = {
            {1, 2, 0},       // Block 1 (Grass)
            {0, 0, 0},      // Block 2 (Dirt)
            {2, 2, 2},      // Block 2 (Moss)
    };

    // Bind VAO
    this->VAO.bind();

    // Create VAO and VBO with vertices and UV data
    this->VAO.linkVertices(this->vertices);
    this->VAO.linkIndices(this->indices);
    this->VAO.linkFaces(faces);
//    this->VAO.linkUV(this->uv);


    // Unbind VAO
    this->VAO.unbind();
}

void Chunk::bind() const {
    this->VAO.bind();
}

void Chunk::draw() const
{
    this->bind();
//    glDrawArrays(GL_TRIANGLES, 0, this->vertices.size() / 3);
//    glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
//    glDrawArraysInstanced(GL_TRIANGLES, 0, this->vertices.size() / 3, 1);
    glDrawElementsInstanced(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0, 3);
}
