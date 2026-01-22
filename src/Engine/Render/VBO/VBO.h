#ifndef VBO_H
#define VBO_H

#define GLM_ENABLE_EXPERIMENTAL

#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Vertex.h"

class VBO {
    GLuint ID{};

    public:
        VBO();
        ~VBO();

        template<typename T>
        void addData(const std::vector<T> &v) const;

        void bind() const;
        void unbind() const;
};

#endif //VBO_H
