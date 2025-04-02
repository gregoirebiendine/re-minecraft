#include "Drawer.h"

Drawer::Drawer()
= default;

void Drawer::draw() const
{
    // this->texture.bind();
    this->chunk.draw();
}

Shader &Drawer::getShader() {
    // return this->shaders;
}

