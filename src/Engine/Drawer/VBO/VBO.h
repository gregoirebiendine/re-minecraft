#ifndef VBO_H
#define VBO_H

#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class VBO {
    public:
        VBO();
        ~VBO();

        GLuint ID = -1;

        void addData(const std::vector<GLfloat> &data) const;
        void bind() const;
        void unbind() const;
};

#endif //VBO_H
