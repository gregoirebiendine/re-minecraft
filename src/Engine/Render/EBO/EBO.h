
#ifndef RE_MINECRAFT_EBO_H
#define RE_MINECRAFT_EBO_H

#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class EBO {

    public:
        EBO();
        ~EBO();

        GLuint ID = -1;

        void addData(const std::vector<GLuint> &indices) const;
        void bind() const;
        void unbind() const;
};


#endif //RE_MINECRAFT_EBO_H
