#ifndef SHADER_H
#define SHADER_H

#include <filesystem>
#include <fstream>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
    GLuint ID;

    GLint modelMatrixUniform{-1};
    GLint viewMatrixUniform{-1};
    GLint projectionMatrixUniform{-1};

    static std::string loadFile(const std::string& path);
    static void checkCompileErrors(GLuint shader, const char* name);

    public:
        Shader(const std::string& vertexPath, const std::string& fragPath);
        ~Shader();

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;

        void use() const;
        void setUniformInt(const char *name, int value) const;
        void setUniformFloat(const char *name, float value) const;
        void setUniformVec3(const char *name, glm::vec3 value) const;
        void setUniformVec4(const char *name, glm::vec4 value) const;
        void setUniformMat4(const char *name, glm::mat4 value) const;

        void setModelMatrix(const glm::mat4& modelMatrix) const;
        void setViewMatrix(const glm::mat4& viewMatrix) const;
        void setProjectionMatrix(const glm::mat4& projectionMatrix) const;
};

#endif
