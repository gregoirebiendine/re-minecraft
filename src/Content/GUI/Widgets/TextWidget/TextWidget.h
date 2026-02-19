#ifndef FARFIELD_TEXTWIDGET_H
#define FARFIELD_TEXTWIDGET_H

#include <string>
#include <functional>

#include "AWidget.h"
#include "RGBA.h"
#include "Font.h"

enum class TextAlign { Left, Center, Right };

class TextWidget : public AWidget
{
    const Font& font;
    const TextureId& fontTexId;

    std::string cachedText;
    RGBA color{255, 255, 255, 1.f};
    TextAlign align{TextAlign::Left};
    float scale{1.f};
    std::function<std::string()> textBinding;

    public:
        TextWidget(const Font& font, glm::vec2 position, float scale = 1.f);

        void setText(const std::string& text);
        void bind(std::function<std::string()> fn);
        void setAlignment(TextAlign a);

        void tick() override;
        void buildSelf(std::vector<GuiVertex>& out, glm::vec2 abs) override;
};

#endif
