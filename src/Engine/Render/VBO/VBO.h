#ifndef VBO_H
#define VBO_H

#define GLM_ENABLE_EXPERIMENTAL

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vector_angle.hpp>

class VBO {
    public:
        VBO();
        ~VBO();

        GLuint ID = -1;

        void addData(const std::vector<GLfloat> &data) const;
        void addData(const std::vector<GLuint> &data) const;
        void addData(const std::vector<GLint> &data) const;
        void addData(const std::vector<glm::vec3> &data) const;
        void addTransformMatrix() const;
        void bind() const;
        void unbind() const;
};

#endif //VBO_H
