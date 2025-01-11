#ifndef DRAWER_H
#define DRAWER_H

#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
        Shaders &getShader();
};


#endif //DRAWER_H
