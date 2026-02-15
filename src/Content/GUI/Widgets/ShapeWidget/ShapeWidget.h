#ifndef FARFIELD_SHAPEWIDGET_H
#define FARFIELD_SHAPEWIDGET_H

#include "AWidget.h"

class ShapeWidget : public AWidget
{
    std::vector<GuiVertex> vertices;

    public:
        explicit ShapeWidget(std::vector<GuiVertex> verts);

        void setVertices(std::vector<GuiVertex> verts);
        void buildSelf(std::vector<GuiVertex>& out, glm::vec2 abs) override;
};

#endif
