#include "GUI.h"
#include "OutlineVertices.h"
#include "CrosshairVertices.h"
#include "ShapeWidget.h"
#include "TextWidget.h"

GUI::GUI(const Font& _font, const TextureRegistry& _textureRegistry, Settings& _settings) :
    font(_font),
    textureRegistry(_textureRegistry),
    settings(_settings),
    guiShader("/resources/shaders/UI/"),
    outlineShader("/resources/shaders/Outline/")
{
    const auto& viewportSize = this->settings.getViewportSize();
    const auto vpX = static_cast<float>(viewportSize.x);
    const auto vpY = static_cast<float>(viewportSize.y);

    // Set shader uniforms
    this->guiShader.use();
    this->guiShader.setUniformInt("Textures", 0);

    // Build widget tree
    this->root = std::make_unique<PanelWidget>();

    // Crosshair
    this->root->addChild(std::make_unique<ShapeWidget>(getCrosshairVertices(viewportSize)));

    // Debug panel
    this->debugPanel = dynamic_cast<PanelWidget*>(this->root->addChild(
        std::make_unique<PanelWidget>(
            glm::vec2{0.f, 0.f},
            glm::vec2{450.f, 140.f},
            RGBA{50, 50, 50, 0.25f}
        )
    ));

    auto makeBoundText = [&](const float y, std::function<std::string()> fn) {
        auto t = std::make_unique<TextWidget>(this->font, glm::vec2{10.f, y});
        t->bind(std::move(fn));
        return t;
    };

    this->debugPanel->addChild(makeBoundText(5.f, [this] {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << this->settings.getCurrentFps();
        return "FPS: " + ss.str();
    }));

    this->debugPanel->addChild(makeBoundText(30.f, [this] {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2)
           << this->currentPos.x << ", "
           << this->currentPos.y << ", "
           << this->currentPos.z;
        return "Pos: " + ss.str();
    }));

    this->debugPanel->addChild(makeBoundText(55.f, [this] {
        const ChunkPos cp{
            static_cast<int>(this->currentPos.x) / 16,
            static_cast<int>(this->currentPos.y) / 16,
            static_cast<int>(this->currentPos.z) / 16
        };
        return "Chunk: " + std::to_string(cp.x) + ", "
                         + std::to_string(cp.y) + ", "
                         + std::to_string(cp.z);
    }));

    this->debugPanel->addChild(makeBoundText(80.f, [this] {
        return "Facing: " + DirectionUtils::forwardVectorToCardinal(this->currentForward);
    }));

    this->debugPanel->addChild(makeBoundText(105.f, [this] {
        return "Block: " + this->currentSelectedBlock;
    }));

    // Hotbar image
    {
        const auto texId = this->textureRegistry.getByName("hotbar");
        const auto& slot = this->textureRegistry.getSlot(texId);
        const float w = static_cast<float>(slot.width) * 1.5f;
        const float h = static_cast<float>(slot.height) * 1.5f;
        const float x = vpX / 2.f - w * 0.5f;
        const float y = vpY - h - 20.f;
        this->root->addChild(std::make_unique<ImageWidget>(
            this->textureRegistry, "hotbar", glm::vec2{x, y}, glm::vec2{w, h}
        ));
    }

    // Hotbar selection
    {
        const auto texId = this->textureRegistry.getByName("hotbar_selection");
        const auto& slot = this->textureRegistry.getSlot(texId);
        const float w = static_cast<float>(slot.width) * 1.5f;
        const float h = static_cast<float>(slot.height) * 1.5f;
        const float x = vpX / 2.f - w * 0.5f;
        const float y = vpY - h - 20.f;
        this->hotbarSelection = dynamic_cast<ImageWidget*>(
            this->root->addChild(std::make_unique<ImageWidget>(
                this->textureRegistry, "hotbar_selection", glm::vec2{x, y}, glm::vec2{w, h}
            ))
        );
    }

    // Initial build
    this->rebuildVertexBuffer();

    // Upload Block Outline data
    this->outlineVao.bind();
    this->outlineVao.storeOutlineData(OUTLINE_VERTICES);
    this->outlineVao.unbind();
}

glm::mat4 GUI::getGUIProjectionMatrix() const
{
    const auto& viewportSize = this->settings.getViewportSize();

    return glm::ortho(
        0.0f, static_cast<float>(viewportSize.x),
        static_cast<float>(viewportSize.y), 0.0f,
        -1.f, 1.0f
    );
}

void GUI::onHotbarSlotChanged(const int slot) const
{
    if (!this->hotbarSelection)
        return;

    const auto& viewportSize = this->settings.getViewportSize();
    const auto vpX = static_cast<float>(viewportSize.x);
    const auto vpY = static_cast<float>(viewportSize.y);

    const auto texId = this->textureRegistry.getByName("hotbar_selection");
    const auto& texSlot = this->textureRegistry.getSlot(texId);
    const float w = static_cast<float>(texSlot.width) * 1.5f;
    const float h = static_cast<float>(texSlot.height) * 1.5f;

    // Offset selection by slot index (each slot is w wide)
    const float x = vpX / 2.f - w * 0.5f + static_cast<float>(slot) * w;
    const float y = vpY - h - 20.f;

    this->hotbarSelection->setPosition(glm::vec2{x, y});
}

void GUI::toggleDebugPanel() const
{
    this->debugPanel->setVisible(!this->debugPanel->isVisible());
}

void GUI::render(const glm::vec3& pos, const glm::vec3& forward, const std::string& selectedBlockName)
{
    // Update per-frame data for bindings
    this->currentPos = pos;
    this->currentForward = forward;
    this->currentSelectedBlock = selectedBlockName;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Evaluates bindings
    this->root->tick();

    // Rebuild VBO only if something changed
    if (this->root->isAnyDirty())
        this->rebuildVertexBuffer();

    // Draw
    this->guiShader.use();
    this->guiShader.setProjectionMatrix(this->getGUIProjectionMatrix());

    this->guiVao.bind();
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(this->vertexBuffer.size()));
    this->guiVao.unbind();

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void GUI::renderBlockOutline(const glm::mat4& v, const glm::mat4& p, const glm::vec3& pos)
{
    glDisable(GL_CULL_FACE);
    glPolygonOffset(-1, -1);

    this->outlineShader.use();
    this->outlineShader.setModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(pos)));
    this->outlineShader.setViewMatrix(v);
    this->outlineShader.setProjectionMatrix(p);

    this->outlineVao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 288);
    this->outlineVao.unbind();

    glPolygonOffset(0, 0);
    glEnable(GL_CULL_FACE);
}

void GUI::rebuildVertexBuffer()
{
    this->vertexBuffer.clear();
    this->root->build(this->vertexBuffer, {0.f, 0.f});
    this->root->clearDirty();

    this->guiVao.bind();
    this->guiVao.storeGuiData(this->vertexBuffer);
    this->guiVao.unbind();
}
