
#include "VBO.h"

VBO::VBO()
{
    glGenBuffers(1, &this->ID);
}

VBO::~VBO()
{
    glDeleteBuffers(1, &this->ID);
}

void VBO::addData(const std::vector<GLfloat> &data) const {
    this->bind();
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat), data.data(), GL_STATIC_DRAW);
}

void VBO::addData(const std::vector<GLuint> &data) const {
    this->bind();
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLuint), data.data(), GL_STATIC_DRAW);
}

void VBO::addData(const std::vector<GLint> &data) const {
    this->bind();
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLint), data.data(), GL_STATIC_DRAW);
}


void VBO::addData(const std::vector<glm::vec3> &data) const
{
    this->bind();
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(glm::vec3), data.data(), GL_STATIC_DRAW);
}

void VBO::addTransformMatrix() const
{
    const glm::mat4 faceMatrices[6] = {
        glm::translate(glm::mat4(1.0f), glm::vec3( 0.0f,  0.0f,  0.5f)), // Avant
        glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, -0.5f)), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)), // Arrière
        glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.5f)), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), // Gauche
        glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.5f)), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), // Droite
        glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.5f)), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)), // Haut
        glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.5f)), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),  // Bas
    };

    this->bind();
    glBufferData(GL_ARRAY_BUFFER, sizeof(faceMatrices), faceMatrices, GL_STATIC_DRAW);
}

void VBO::bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, this->ID);
}

void VBO::unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}