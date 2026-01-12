#include "TextureManager.h"

TextureManager::TextureManager() {
    const std::filesystem::path texturePath = std::filesystem::current_path().parent_path().append("/resources/textures/");
    const std::vector<std::string> paths = {
        "dirt.png",
        "grass_block_side.png",
        "grass_block_top.png",
        "grass_block_top.png",
        "cobble.png",
        "stone.png",
        "oak_plank.png",
    };

    // Create OpenGL texture
    glGenTextures(1, &this->ID);
    glBindTexture(GL_TEXTURE_2D_ARRAY, this->ID);

    // Create 2D array of textures with W=32 and H=32
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, 32, 32, static_cast<int>(paths.size()));

    int w, h, channels;
    for (unsigned int i = 0; i < paths.size(); i++) {
        stbi_uc* tex = stbi_load((texturePath.string() + paths[i]).c_str(), &w, &h, &channels, 0);

        if (!tex)
            throw std::runtime_error(stbi_failure_reason());
        if (w != 32 || h != 32)
            throw std::runtime_error("Texture " + paths[i] + " is not 32 pixel wide");

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, 32, 32, 1, GL_RGBA, GL_UNSIGNED_BYTE, tex);
        stbi_image_free(tex);
    }

    // Set texture loading filter (nearest to keep pixels)
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    glActiveTexture(GL_TEXTURE0);
    // glUniform1i(glGetUniformLocation(this->ID, "tex0"), 0);
}

TextureManager::~TextureManager() {
    glDeleteTextures(1, &this->ID);
}

void TextureManager::activate()
{
    glActiveTexture(GL_TEXTURE0);
}

void TextureManager::bind() const {
    glBindTexture(GL_TEXTURE_2D_ARRAY, this->ID);
}