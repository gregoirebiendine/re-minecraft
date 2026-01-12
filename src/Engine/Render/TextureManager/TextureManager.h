#ifndef RE_MINECRAFT_TEXTUREMANAGER_H
#define RE_MINECRAFT_TEXTUREMANAGER_H

#include <stdexcept>
#include <vector>
#include <iostream>
#include <filesystem>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stbi/stb_image.h>

class TextureManager
{
    GLuint ID{};

    public:
        TextureManager();
        ~TextureManager();

        static void activate() ;
        void bind() const;
};

#endif //RE_MINECRAFT_TEXTUREMANAGER_H