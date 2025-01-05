#ifndef DRAWER_H
#define DRAWER_H

#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Shaders.h"
#include "VBO.h"
#include "EBO.h"
#include "VAO.h"
#include "Texture.h"
#include "Chunk.h"


class Drawer {
    Shaders shaders;
    Texture texture;
    Chunk chunk;

    public:
        Drawer();

        void draw() const;
};


#endif //DRAWER_H
