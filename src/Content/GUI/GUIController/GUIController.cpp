#include "GUIController.h"
#include "OutlineVertices.h"

GUIController::GUIController(const Font& _font, const TextureRegistry& _textureRegistry, const Settings& _settings) :
    panel(_font, _textureRegistry, _settings),
    shader("/resources/shaders/Outline/")
{
    this->vao.bind();
    this->vao.storeOutlineData(OUTLINE_VERTICES);
    this->vao.unbind();
}

void GUIController::onHotbarSlotChanged(const int slot) const
{
    // if (!this->hotbarSelection)
    //     return;
    //
    // const auto& viewportSize = this->settings.getViewportSize();
    // const auto vpX = static_cast<float>(viewportSize.x);
    // const auto vpY = static_cast<float>(viewportSize.y);
    //
    // const auto texId = this->textureRegistry.getByName("hotbar_selection");
    // const auto& texSlot = this->textureRegistry.getSlot(texId);
    // const float w = static_cast<float>(texSlot.width) * 1.5f;
    // const float h = static_cast<float>(texSlot.height) * 1.5f;
    //
    // // Offset selection by slot index (each slot is w wide)
    // const float x = vpX / 2.f - w * 0.5f + static_cast<float>(slot) * w;
    // const float y = vpY - h - 20.f;
    //
    // this->hotbarSelection->setPosition(glm::vec2{x, y});
}

void GUIController::toggleDebugPanel() const
{
    this->panel.toggleDebugPanel();
}

void GUIController::update(const glm::vec3 &pos, const glm::vec3 &forward, const std::string &selectedBlockName)
{
    this->panel.update(pos, forward, selectedBlockName);
}

void GUIController::render()
{
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    this->panel.render();

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void GUIController::renderBlockOutline(const glm::mat4& v, const glm::mat4& p, const glm::vec3& pos)
{
    glEnable(GL_MULTISAMPLE);
    glDisable(GL_CULL_FACE);
    glPolygonOffset(-1, -1);

    this->shader.use();
    this->shader.setModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(pos)));
    this->shader.setViewMatrix(v);
    this->shader.setProjectionMatrix(p);

    this->vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 288);
    this->vao.unbind();

    glPolygonOffset(0, 0);
    glEnable(GL_CULL_FACE);
    glDisable(GL_MULTISAMPLE);
}