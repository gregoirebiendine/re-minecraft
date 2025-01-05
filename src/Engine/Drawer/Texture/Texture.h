
#ifndef RE_MINECRAFT_TEXTURE_H
#define RE_MINECRAFT_TEXTURE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stbi/stb_image.h>

class Texture {
    public:
        GLuint ID = -1;

        Texture(const char *path);
        ~Texture();

        void bind() const;
};


#endif //RE_MINECRAFT_TEXTURE_H
