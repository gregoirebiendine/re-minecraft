#ifndef SHADERS_H
#define SHADERS_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Shaders {
    static std::string loadFile(const char *path);

    public:
        GLuint ID;

        Shaders(const char* vertexSource, const char* fragSource);
        ~Shaders();
        void use() const;
};

#endif //SHADERS_H
