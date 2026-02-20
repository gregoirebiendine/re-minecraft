#include "AWidget.h"

void AWidget::markDirty()
{
    this->dirty = true;
}

bool AWidget::isDirty() const
{
    return this->dirty;
}

void AWidget::clearDirty()
{
    this->dirty = false;
    for (const auto& child : this->children)
        child->clearDirty();
}

bool AWidget::isAnyDirty() const
{
    if (!this->dirty && !this->visible)
        return false;

    if (this->dirty)
        return true;

    for (const auto& child : this->children)
        if (child->isAnyDirty())
            return true;
    return false;
}

void AWidget::setPosition(const glm::vec2 pos)
{
    this->position = pos;
    this->markDirty();
}

glm::vec2 AWidget::getPosition() const
{
    return this->position;
}

void AWidget::bindPosition(std::function<glm::vec2()> fn)
{
    this->positionBinding = std::move(fn);
}

void AWidget::setSize(const glm::vec2 s)
{
    this->size = s;
    this->markDirty();
}

glm::vec2 AWidget::getSize() const
{
    return this->size;
}

void AWidget::setVisible(const bool v)
{
    this->visible = v;
    this->markDirty();
}

void AWidget::bindVisibility(std::function<bool()> fn)
{
    this->visibilityBinding = std::move(fn);
}

bool AWidget::isVisible() const
{
    return this->visible;
}

AWidget* AWidget::addChild(std::unique_ptr<AWidget> child)
{
    AWidget* ptr = child.get();
    this->children.push_back(std::move(child));
    return ptr;
}

void AWidget::clearChildren()
{
    this->children.clear();
    this->markDirty();
}

void AWidget::build(std::vector<GuiVertex>& out, const glm::vec2 parentOffset)
{
    if (!this->visible)
        return;

    const glm::vec2 abs = parentOffset + this->position;

    this->buildSelf(out, abs);
    for (const auto& child : this->children)
        child->build(out, abs);
}

void AWidget::buildMsdfTree(std::vector<MSDFVertex> &out, const glm::vec2 parentOffset)
{
    if (!this->visible)
        return;

    const glm::vec2 abs = parentOffset + this->position;

    this->buildMsdf(out, abs);
    for (const auto& child : this->children)
        child->buildMsdfTree(out, abs);
}
void AWidget::tick()
{
    if (this->positionBinding) {
        const auto newPos = this->positionBinding();
        if (newPos != this->position) {
            this->position = newPos;
            this->markDirty();
        }
    }

    if (this->visibilityBinding) {
        const bool newVis = this->visibilityBinding();
        if (newVis != this->visible) {
            this->visible = newVis;
            this->markDirty();
        }
    }

    for (const auto& child : this->children)
        child->tick();
}
