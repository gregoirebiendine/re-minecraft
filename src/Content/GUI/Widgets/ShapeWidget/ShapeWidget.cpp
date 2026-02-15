#include "ShapeWidget.h"

ShapeWidget::ShapeWidget(std::vector<GuiVertex> verts) :
    vertices(std::move(verts))
{
}

void ShapeWidget::setVertices(std::vector<GuiVertex> verts)
{
    this->vertices = std::move(verts);
    this->markDirty();
}

void ShapeWidget::buildSelf(std::vector<GuiVertex>& out, const glm::vec2 abs)
{
    for (auto v : this->vertices) {
        v.position += abs;
        out.push_back(v);
    }
}
