#include "Drawer.h"

Drawer::Drawer() :
    shaders("../resources/Shaders/VertexShader.vert","../resources/Shaders/FragmentShader.frag"),
    texture("../resources/Textures/grass.jpg")
{
    // Void
}

void Drawer::draw() const
{
    this->texture.bind();
    this->chunk.draw();
}

