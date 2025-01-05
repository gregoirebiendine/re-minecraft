
#include "Atlas.h"

Atlas::Atlas() {
    int width, height, chan;

    unsigned char *image = stbi_load("../resources/textures/atlas.png", &width, &height, &chan, 0);

    if (!image)
        throw std::runtime_error(stbi_failure_reason());

    glGenTextures(1, &this->ID);

    glBindTexture(GL_TEXTURE_2D, this->ID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(image);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(this->ID, "tex0"), 0);

    glBindTexture(GL_TEXTURE_2D, this->ID);
}

Atlas::~Atlas() {
    glDeleteTextures(1, &this->ID);
}

void Atlas::bind() const {
    glBindTexture(GL_TEXTURE_2D, this->ID);
}