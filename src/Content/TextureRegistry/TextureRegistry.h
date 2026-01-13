#ifndef RE_MINECRAFT_TEXTUREREGISTRY_H
#define RE_MINECRAFT_TEXTUREREGISTRY_H

#include <stdexcept>
#include <vector>
#include <iostream>
#include <filesystem>
#include <unordered_map>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stbi/stb_image.h>

namespace fs = std::filesystem;
using TextureId = unsigned short;

class TextureRegistry
{
    GLuint ID{};

    std::vector<std::string> textures;
    std::unordered_map<std::string, TextureId> nameToTextureId;

    public:
        static constexpr std::string MISSING = "MISSING";

        TextureRegistry();
        ~TextureRegistry();

        void createTextures();

        TextureId registerTexture(const std::string& name, const std::string& path);
        const std::string& get(TextureId id) const;
        TextureId getByName(const std::string& name) const;

        void bind() const;
};

#endif //RE_MINECRAFT_TEXTUREREGISTRY_H