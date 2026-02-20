#ifndef FARFIELD_AWIDGET_H
#define FARFIELD_AWIDGET_H

#include <memory>
#include <vector>
#include <functional>
#include <glm/glm.hpp>

#include "VAOVertices.h"

class AWidget
{
    glm::vec2 position{0.f};
    glm::vec2 size{0.f};
    bool visible{true};
    bool dirty{true};

    std::function<glm::vec2()> positionBinding;
    std::function<bool()> visibilityBinding;

    std::vector<std::unique_ptr<AWidget>> children;

    public:
        virtual ~AWidget() = default;

        [[nodiscard]] bool isDirty() const;
        [[nodiscard]] bool isAnyDirty() const;
        void markDirty();
        void clearDirty();

        void setPosition(glm::vec2 pos);
        [[nodiscard]] glm::vec2 getPosition() const;
        void bindPosition(std::function<glm::vec2()> fn);

        void setSize(glm::vec2 s);
        [[nodiscard]] glm::vec2 getSize() const;

        void setVisible(bool v);
        void bindVisibility(std::function<bool()> fn);
        [[nodiscard]] bool isVisible() const;

        AWidget* addChild(std::unique_ptr<AWidget> child);
        void clearChildren();

        void build(std::vector<GuiVertex>& out, glm::vec2 parentOffset);
        virtual void buildSelf(std::vector<GuiVertex>& out, glm::vec2 absolutePos) = 0;

        void buildMsdfTree(std::vector<MSDFVertex>& out, glm::vec2 parentOffset);
        virtual void buildMsdf(std::vector<MSDFVertex>& out, glm::vec2 abs) {}

        virtual void tick();
};


#endif