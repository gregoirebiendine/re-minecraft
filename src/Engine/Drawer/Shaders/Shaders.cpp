
#include "Shaders.h"

std::string Shaders::loadFile(const char* path)
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

Shaders::Shaders(const char* vertexSource, const char* fragSource)
{
    const std::string vertexContent = Shaders::loadFile(vertexSource);
    const std::string fragContent = Shaders::loadFile(fragSource);
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

Shaders::~Shaders() {
    glDeleteProgram(this->ID);
}

void Shaders::use() const
{
    glUseProgram(this->ID);
}
