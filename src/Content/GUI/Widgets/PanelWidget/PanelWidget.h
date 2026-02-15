#ifndef FARFIELD_PANELWIDGET_H
#define FARFIELD_PANELWIDGET_H

#include <optional>

#include "AWidget.h"
#include "RGBA.h"

class PanelWidget : public AWidget
{
    std::optional<RGBA> backgroundColor;

    public:
        PanelWidget() = default;
        PanelWidget(glm::vec2 position, glm::vec2 size, RGBA bgColor);

        void buildSelf(std::vector<GuiVertex>& out, glm::vec2 abs) override;
};

#endif
