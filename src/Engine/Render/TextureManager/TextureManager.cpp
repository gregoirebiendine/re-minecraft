#include "TextureManager.h"

TextureManager::TextureManager() {
    int width, height, chan;
    std::vector<stbi_uc*> images;
    const auto currPath = std::filesystem::current_path().parent_path();
    const auto texturePath = std::filesystem::path(currPath/"resources/textures/");

    for (auto const& dir_entry : std::filesystem::directory_iterator{texturePath})
    {
        if (dir_entry.path().string().contains("atlas"))
            continue;
        stbi_uc* tex = stbi_load(dir_entry.path().string().c_str(), &width, &height, &chan, 0);

        if (!tex)
            throw std::runtime_error(stbi_failure_reason());
        if (width != 32 || height != 32)
            throw std::runtime_error("Texture " + dir_entry.path().string() + " is not 32 pixel wide");

        images.push_back(tex);
    }

    for (auto const& img : images)
        stbi_image_free(img);


    // Create OpenGL texture
    // glGenTextures(1, &this->ID);
    // glBindTexture(GL_TEXTURE_2D_ARRAY, this->ID);
    //
    // // Set texture loading filter (nearest to keep pixels)
    // glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //
    // glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, 32, 32, 2);
    // glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    //
    // stbi_image_free(image);
    //
    // glActiveTexture(GL_TEXTURE0);
    // glUniform1i(glGetUniformLocation(this->ID, "tex0"), 0);
    //
    // glBindTexture(GL_TEXTURE_2D, this->ID);
}

TextureManager::~TextureManager() {
    // glDeleteTextures(1, &this->ID);
}

void TextureManager::bind() const {
    glBindTexture(GL_TEXTURE_2D_ARRAY, this->ID);
}