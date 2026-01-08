#ifndef SHADER_H
#define SHADER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
    static std::string loadFile(const std::string& path);

    public:
        GLuint ID;

        Shader(const std::string& vertexPath, const std::string& fragPath);
        ~Shader();

        void use() const;
        void setUniformMat4(const char *name, glm::mat4 value) const;
};

#endif
