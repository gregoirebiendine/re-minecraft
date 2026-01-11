#ifndef SHADER_H
#define SHADER_H

#include <filesystem>
#include <fstream>
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
        void setUniformInt(const char *name, int value) const;
        void setUniformFloat(const char *name, float value) const;
        void setUniformVec3(const char *name, glm::vec3 value) const;
        void setUniformVec4(const char *name, glm::vec4 value) const;
        void setUniformMat4(const char *name, glm::mat4 value) const;
};

#endif
