#include "Drawer.h"

Drawer::Drawer() :
    shaders("../resources/Shaders/VertexShader.vert","../resources/Shaders/FragmentShader.frag"),
    texture("../resources/Textures/atlas.png")
{

}

void Drawer::draw() const
{
    this->texture.bind();
    this->chunk.draw();
}

Shaders &Drawer::getShader() {
    return this->shaders;
}

