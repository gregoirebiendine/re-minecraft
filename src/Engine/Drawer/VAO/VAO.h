
#ifndef RE_MINECRAFT_VAO_H
#define RE_MINECRAFT_VAO_H

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "VBO.h"

class VAO {
    GLuint ID = -1;

    public:
        VAO();
        ~VAO();

        void linkAttrib(VBO &VBO, GLuint layout, GLint size, GLenum type, GLsizei stride, void *offset) const;
        void linkSquareAttrib(VBO &VBO) const;
        void bind() const;
        void unbind() const;
};


#endif //RE_MINECRAFT_VAO_H
