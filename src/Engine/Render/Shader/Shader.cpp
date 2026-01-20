
#include "Shader.h"

std::string Shader::loadFile(const std::string& path)
{
    std::ifstream file(path);

    if (!file.is_open())
        throw std::runtime_error("Failed to load file " + path);

    std::string content;
    file.seekg(0, std::ios::end);
    content.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(&content[0], content.size());
    file.close();
    return content;
}

Shader::Shader(const std::string& vertexPath, const std::string& fragPath)
{
    const std::string vertexContent = loadFile(vertexPath);
    const std::string fragContent = loadFile(fragPath);

    const char *vertexShaderSource = vertexContent.c_str();
    const char *fragmentShaderSource = fragContent.c_str();

    // Create Vertex Shader of count 1, that read from vertexShaderSource (basic shape)
    const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    checkCompileErrors(vertexShader, "VERTEX");

    // Create Fragment Shader of count 1, that read from fragmentShaderSource (color & rasterization)
    const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    checkCompileErrors(fragmentShader, "FRAGMENT");

    // Create a Shader Program and link Vertex/Fragment shaders
    this->ID = glCreateProgram();
    glAttachShader(this->ID, vertexShader);
    glAttachShader(this->ID, fragmentShader);
    glLinkProgram(this->ID);

    // Delete Vertex/Fragment shaders because they are combined into a Program
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::~Shader() {
    glDeleteProgram(this->ID);
}

void Shader::use() const
{
    glUseProgram(this->ID);
}

void Shader::checkCompileErrors(const GLuint shader, const char* name)
{
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

        std::string infoLog(logLength, '\0');
        glGetShaderInfoLog(shader, logLength, nullptr, infoLog.data());
        glDeleteShader(shader);

        std::cerr << "Shader " << name << " compilation failed : " << infoLog << std::endl;
        throw std::runtime_error(infoLog);
    }
}

void Shader::setUniformInt(const char *name, const int value) const {
    const int loc = glGetUniformLocation(this->ID, name);
    glUniform1i(loc, value);
}

void Shader::setUniformFloat(const char *name, const float value) const {
    const int loc = glGetUniformLocation(this->ID, name);
    glUniform1f(loc, value);
}

void Shader::setUniformVec3(const char *name, const glm::vec3 value) const {
    const int loc = glGetUniformLocation(this->ID, name);
    glUniform3f(loc, value.x, value.y, value.z);
}

void Shader::setUniformVec4(const char *name, const glm::vec4 value) const {
    const int loc = glGetUniformLocation(this->ID, name);
    glUniform4f(loc, value.x, value.y, value.z, value.w);
}

void Shader::setUniformMat4(const char *name, glm::mat4 value) const
{
    const int loc = glGetUniformLocation(this->ID, name);
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
}