#include "PanelWidget.h"

PanelWidget::PanelWidget(const glm::vec2 position, const glm::vec2 size, const RGBA bgColor) :
    backgroundColor(bgColor)
{
    this->setPosition(position);
    this->setSize(size);
}

void PanelWidget::buildSelf(std::vector<GuiVertex>& out, const glm::vec2 abs)
{
    if (!this->backgroundColor.has_value())
        return;

    const glm::vec4 color = this->backgroundColor.value();
    const glm::vec2 s = this->getSize();
    const float x = abs.x;
    const float y = abs.y;
    const float x1 = x + s.x;
    const float y1 = y + s.y;

    out.insert(out.end(), {
        {{x, y},   {-1.f, -1.f}, color},
        {{x, y1},  {-1.f, -1.f}, color},
        {{x1, y1}, {-1.f, -1.f}, color},

        {{x, y},   {-1.f, -1.f}, color},
        {{x1, y1}, {-1.f, -1.f}, color},
        {{x1, y},  {-1.f, -1.f}, color},
    });
}
