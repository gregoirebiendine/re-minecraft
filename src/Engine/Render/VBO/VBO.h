#ifndef VBO_H
#define VBO_H

#define GLM_ENABLE_EXPERIMENTAL

#include <vector>
#include <variant>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "Vertex.h"

class VBO {
    GLuint ID{};

    public:
        VBO();
        ~VBO();

        template<typename T>
        void addData(const std::vector<T> &v) const;

        void storeBlockData(const std::vector<Vertex> &v) const;

        void bind() const;
        void unbind() const;
};

#endif //VBO_H
