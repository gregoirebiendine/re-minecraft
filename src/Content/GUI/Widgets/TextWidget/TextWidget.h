#ifndef FARFIELD_TEXTWIDGET_H
#define FARFIELD_TEXTWIDGET_H

#include <string>
#include <functional>

#include "AWidget.h"
#include "MsdfFont.h"
#include "RGBA.h"

enum class TextAlign { Left, Center, Right };

class TextWidget : public AWidget
{
    const MsdfFont& font;

    float scale{1.f};
    bool shadow{true};
    TextAlign align{TextAlign::Left};
    RGBA color = RGBA::fromRGB(255, 255, 255, 1.f);

    std::string cachedText;
    std::function<std::string()> textBinding;

    void buildGlyph(std::vector<MSDFVertex>& out, float curX, float y, float offset, RGBA c) const;

    public:
        TextWidget(const MsdfFont& font, glm::vec2 position, float scale = 0.5f);

        void setText(const std::string& text);
        void setAlignment(TextAlign a);
        void setShadow(bool _shadow);

        void bind(std::function<std::string()> fn);

        void tick() override;

        void buildSelf(std::vector<GuiVertex>& out, glm::vec2 abs) override {};
        void buildMsdf(std::vector<MSDFVertex>& out, glm::vec2 abs) override;
};

#endif
