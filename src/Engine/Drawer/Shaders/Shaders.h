#ifndef SHADERS_H
#define SHADERS_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shaders {
    static std::string loadFile(const char *path);

    public:
        GLuint ID;

        Shaders(const char* vertexSource, const char* fragSource);
        ~Shaders();

        void use() const;
        void setUniformMat4(const char *name, glm::mat4 value) const;
};

#endif //SHADERS_H
