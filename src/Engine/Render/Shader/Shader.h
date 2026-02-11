#ifndef FARFIELD_SHADER_H
#define FARFIELD_SHADER_H

#include <filesystem>
#include <fstream>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Utils.h"

namespace fs = std::filesystem;

class Shader {
    GLuint ID;

    GLint modelMatrixUniform{-1};
    GLint viewMatrixUniform{-1};
    GLint projectionMatrixUniform{-1};

    static std::string loadFile(const std::string& path);
    static void checkCompileErrors(GLuint shader, const char* name);

    public:
        Shader(const std::string& folder);
        ~Shader();

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;

        void use() const;
        void setUniformInt(const char *name, int value) const;
        void setUniformUInt(const char *name, unsigned int value) const;
        void setUniformFloat(const char *name, float value) const;
        void setUniformVec3(const char *name, glm::vec3 value) const;
        void setUniformVec4(const char *name, glm::vec4 value) const;
        void setUniformMat4(const char *name, glm::mat4 value) const;

        void setModelMatrix(const glm::mat4& modelMatrix);
        void setViewMatrix(const glm::mat4& viewMatrix);
        void setProjectionMatrix(const glm::mat4& projectionMatrix);
};

#endif
