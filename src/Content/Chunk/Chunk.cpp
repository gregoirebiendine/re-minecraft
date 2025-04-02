#include "Chunk.h"

Chunk::Chunk()
{
    // Declare base vertices and UVs of a face
    this->vertices = {
        0.0f, 0.0f, 0.0f,       0.0f, 0.0f,         // Bottom Left
        1.0f, 0.0f, 0.0f,       0.25f, 0.0f,        // Bottom Right
        1.0f, 1.0f, 0.0f,       0.25f, 0.25f,       // Top Right
        0.0f, 1.0f, 0.0f,       0.0f, 0.25f,        // Top Left
    };

    // Vertices for left side of a cube
    // this->vertices = {
    //     0.0f, 0.0f, 0.0f,       0.0f, 0.0f,         // Bottom Left
    //     0.0f, 0.0f, 1.0f,       0.25f, 0.0f,        // Bottom Right
    //     0.0f, 1.0f, 1.0f,       0.25f, 0.25f,       // Top Right
    //     0.0f, 1.0f, 0.0f,       0.0f, 0.25f,        // Top Left
    // };

    // Declare indices needed to draw a square (face) with 2 triangles
    this->indices = {
            0, 1, 2, 0, 2, 3,
    };

    // Declare UV offsets per face instance (is equal to atlas texture id)
    this->UVoffsets = {0, 1, 2}; // Is equal to face ID (0: dirt / 1: grass_side / 2: grass_top)

    glm::mat4 faceMatrices[6] = {
        glm::translate(glm::mat4(1.0f), glm::vec3( 0.0f,  0.0f,  0.5f)), // Avant
        glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.5f)), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)), // Arrière
        glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.0f, 0.0f)), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), // Gauche
        glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 0.0f)), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), // Droite
        glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f)), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)), // Haut
        glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f)), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f))  // Bas
    };

    // Bind VAO
    this->VAO.bind();

    // Link datas to VA0 before rendering
    this->VAO.linkVerticesAndUVs(this->vertices);
    this->VAO.linkOffset(this->UVoffsets);
    this->VAO.linkIndices(this->indices);

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

    // Draw 4 instances (faces)
    glDrawElementsInstanced(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0, this->UVoffsets.size());
}
