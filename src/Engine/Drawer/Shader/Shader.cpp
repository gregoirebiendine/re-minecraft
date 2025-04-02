
#include "Shader.h"

std::string Shader::loadFile(const char* path)
{
    std::ifstream file(path);

    if (!file.is_open())
        throw std::runtime_error("Failed to load vertex shader and/or fragment shaders!");

    std::string content;
    file.seekg(0, std::ios::end);
    content.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(&content[0], content.size());
    file.close();
    return content;
}

Shader::Shader()
{
    const std::string vertexContent = Shader::loadFile("../resources/shaders/VertexShader.vert");
    const std::string fragContent = Shader::loadFile("../resources/shaders/FragmentShader.frag");
    const char *vertexShaderSource = vertexContent.c_str();
    const char *fragmentShaderSource = fragContent.c_str();

    if (vertexShaderSource == nullptr || fragmentShaderSource == nullptr)
        throw std::runtime_error("Failed to load vertex shader and/or fragment shaders!");

    // Create Vertex Shader of count 1, that read from vertexShaderSource (basic shape)
    const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    // Create Fragment Shader of count 1, that read from fragmentShaderSource (color & rasterization)
    const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    // Create a Shader Program and link Vertex/Fragment shaders
    this->ID = glCreateProgram();
    glAttachShader(this->ID, vertexShader);
    glAttachShader(this->ID, fragmentShader);
    glLinkProgram(this->ID);

    // Delete Vertex/Fragment shaders because they are combined into a Program
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glUseProgram(this->ID);
}

Shader::~Shader() {
    glDeleteProgram(this->ID);
}

void Shader::use() const
{
    glUseProgram(this->ID);
}

void Shader::setUniformMat4(const char *name, glm::mat4 value) const
{
    const int loc = glGetUniformLocation(this->ID, name);
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
}
