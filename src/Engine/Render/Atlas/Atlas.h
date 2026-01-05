
#ifndef RE_MINECRAFT_ATLAS_H
#define RE_MINECRAFT_ATLAS_H

#include <stdexcept>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stbi/stb_image.h>

class Atlas {
    public:
        GLuint ID{};

        Atlas();
        ~Atlas();

        void bind() const;
};


#endif
