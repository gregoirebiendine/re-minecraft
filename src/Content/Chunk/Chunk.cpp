#include "Chunk.h"

Chunk::Chunk()
{
    // Create squares
    squares.push_back( Square({-0.5f, -0.5f}, {0.5f, 0.5f}) );
    squares.push_back( Square({0.0f, 0.0f}, {0.5f, 0.5f}) );

    // Merge the vertices of all squares
    this->registerSquares();

    // Bind VAO
    this->VAO1.bind();

    // Create EBO and VBO with vertices and indices data
    this->VBO1.addData(this->verts);
    this->EBO1.addData(this->inds);

    // Link attributes of the squares to the VAO
    this->VAO1.linkSquareAttrib(this->VBO1);

    // Unbind all
    this->VAO1.unbind();
    this->VBO1.unbind();
    this->EBO1.unbind();
}

void Chunk::bind() const {
    this->VAO1.bind();
}

void Chunk::registerSquares()
{
    std::vector<GLuint> squareIndices = {0, 2, 1, 0, 3, 2};

    for (auto square : this->squares)
        this->verts.insert(this->verts.begin(), std::begin(square.getVertices()), std::end(square.getVertices()));

    for (GLuint i = 0; i < squares.size() * 4; i += 4) {
        squareIndices = {i, i+2, i+1, i, i+3, i+2};
        this->inds.insert(this->inds.begin(), std::begin(squareIndices), std::end(squareIndices));
    }
}

void Chunk::draw() const
{
    this->bind();
    glDrawElements(GL_TRIANGLES, this->squares.size() * 6, GL_UNSIGNED_INT, 0);
}
