#include "Chunk.h"

Chunk::Chunk()
{
    // Vertices v1
    this->vertices = {
            -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,         // Front face
            -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f,     // Left face
            1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f,     // Back face
            1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f,         // Right face
            -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,         // Top face
            -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f      // Bottom face
    };

    // Indices v1
    this->indices = {
            0, 1, 2, 0, 2, 3,
            4, 5, 6, 4, 6, 7,
            8, 9, 10, 8, 10, 11,
            12, 13, 14, 12, 14, 15,
            16, 17, 18, 16, 18, 19,
            20, 21, 22, 20, 22, 23
    };

    std::vector<glm::vec3> faces = {
            {1, 2, 0},      // Block 1 (Grass)
            {0, 0, 0},      // Block 2 (Dirt)
            {2, 2, 2},      // Block 3 (Moss)
            {0, 2, 2},      // Block 4 (Random)
    };

    // Bind VAO
    this->VAO.bind();

    // Create VAO and VBO with vertices and UV data
    this->VAO.linkVertices(this->vertices);
    this->VAO.linkIndices(this->indices);
    this->VAO.linkFaces(faces);

    // Unbind VAO
    this->VAO.unbind();
}

void Chunk::bind() const {
    this->VAO.bind();
}

void Chunk::draw() const
{
    this->bind();
    glDrawElementsInstanced(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0, 4);
}
