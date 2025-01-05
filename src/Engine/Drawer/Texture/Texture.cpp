
#include "Texture.h"

Texture::Texture(const char *path) {
    int width, height, chan;
    unsigned char *image = stbi_load(path, &width, &height, &chan, 0);

    glGenTextures(1, &this->ID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->ID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(image);

    glUniform1i(glGetUniformLocation(this->ID, "tex0"), 0);
}

Texture::~Texture() {
    glDeleteTextures(1, &this->ID);
}

void Texture::bind() const {
    glBindTexture(GL_TEXTURE_2D, this->ID);
}