#include "Shader.h"

#include "TextureRegistry.h"

Shader::Shader(const std::string& folder)
{
    const auto path = Files::SOURCE_FOLDER.string().append(folder);
    const auto entries = fs::directory_iterator(path);
    std::string vertContent;
    std::string fragContent;

    for (const auto& entry : entries) {
        if (const auto ext = entry.path().filename().extension(); ext == ".vert")
            vertContent = loadFile(entry.path().string());
        else if (ext == ".frag")
            fragContent = loadFile(entry.path().string());
        else
            throw std::runtime_error("[Farfield::Shader] Unsupported shader extension: " + ext.string());
    }

    const char *vertexShaderSource = vertContent.c_str();
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

    this->use();
    this->projectionMatrixUniform = glGetUniformLocation(this->ID, "ProjectionMatrix");
    this->viewMatrixUniform = glGetUniformLocation(this->ID, "ViewMatrix");
    this->modelMatrixUniform = glGetUniformLocation(this->ID, "ModelMatrix");
}

Shader::~Shader() {
    glDeleteProgram(this->ID);
}

void Shader::use() const
{
    glUseProgram(this->ID);
}

std::string Shader::loadFile(const std::string& path)
{
    std::ifstream file(path);

    if (!file.is_open())
        throw std::runtime_error("[Farfield::Shader] Failed to load file " + path);

    std::string content;
    file.seekg(0, std::ios::end);
    content.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(&content[0], static_cast<std::streamsize>(content.size()));
    file.close();
    return content;
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

void Shader::setUniformUInt(const char *name, const unsigned int value) const {
    const int loc = glGetUniformLocation(this->ID, name);
    glUniform1ui(loc, value);
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

void Shader::setModelMatrix(const glm::mat4& modelMatrix)
{
    if (this->modelMatrixUniform == -1)
        this->modelMatrixUniform = glGetUniformLocation(this->ID, "ModelMatrix");
    glUniformMatrix4fv(this->modelMatrixUniform, 1, GL_FALSE, glm::value_ptr(modelMatrix));
}

void Shader::setViewMatrix(const glm::mat4& viewMatrix)
{
    if (this->viewMatrixUniform == -1)
        this->viewMatrixUniform = glGetUniformLocation(this->ID, "ViewMatrix");
    glUniformMatrix4fv(this->viewMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewMatrix));
}

void Shader::setProjectionMatrix(const glm::mat4& projectionMatrix)
{
    if (this->projectionMatrixUniform == -1)
        this->projectionMatrixUniform = glGetUniformLocation(this->ID, "ProjectionMatrix");
    glUniformMatrix4fv(this->projectionMatrixUniform, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
}