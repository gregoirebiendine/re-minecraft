#include "GUIPanel.h"
#include "CrosshairVertices.h"

GUIPanel::GUIPanel(const MsdfFont& _font, const TextureRegistry& _textureRegistry, const ItemRegistry& _itemRegistry, const Viewport& _viewport) :
    font(_font),
    textureRegistry(_textureRegistry),
    itemRegistry(_itemRegistry),
    viewport(_viewport),
    shader("/resources/shaders/UI/"),
    fontShader("/resources/shaders/UI/Font/")
{
    const auto& vpSize = this->viewport.getSize();
    const auto vpX = static_cast<float>(vpSize.x);
    const auto vpY = static_cast<float>(vpSize.y);

    // Set texture uniform
    this->shader.setTextureSamplerId(0);
    this->fontShader.setTextureSamplerId(1);
    this->fontShader.setUniformFloat("ScreenPxRange", this->font.getDistanceRange());

    // Build widget tree
    this->root = std::make_unique<PanelWidget>();

    // Crosshair
    this->crosshair = dynamic_cast<ShapeWidget*>(
        this->root->addChild(std::make_unique<ShapeWidget>(getCrosshairVertices(vpSize)))
    );

    // Debug panel
    {
        this->debugPanel = dynamic_cast<PanelWidget*>(this->root->addChild(
            std::make_unique<PanelWidget>(
                glm::vec2{0.f, 0.f},
                glm::vec2{450.f, 140.f},
                RGBA::fromRGB(50, 50, 50, 0.25f)
            )
        ));
        this->debugPanel->setVisible(false);

        auto makeBoundText = [&](const float y, std::function<std::string()> fn) {
            auto t = std::make_unique<TextWidget>(this->font, glm::vec2{10.f, y});
            t->bind(std::move(fn));
            t->setShadow(false);
            return t;
        };

        this->debugPanel->addChild(makeBoundText(5.f, [this] {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(1) << this->viewport.getSettings().getCurrentFps();
            return "FPS: " + ss.str();
        }));

        this->debugPanel->addChild(makeBoundText(35.f, [this] {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(1)
               << this->currentPos.x << ", "
               << this->currentPos.y << ", "
               << this->currentPos.z;
            return "Pos: " + ss.str();
        }));

        this->debugPanel->addChild(makeBoundText(65.f, [this] {
            const ChunkPos cp{
                static_cast<int>(this->currentPos.x) / 16,
                static_cast<int>(this->currentPos.y) / 16,
                static_cast<int>(this->currentPos.z) / 16
            };
            return "Chunk: " + cp;
        }));

        this->debugPanel->addChild(makeBoundText(95.f, [this] {
            return "Facing: " + DirectionUtils::forwardVectorToCardinal(this->currentForward);
        }));
    }

    // Hotbar
    {
        const auto hotbarTexId = this->textureRegistry.getByName("hotbar");
        const auto& hotbarSlot = this->textureRegistry.getSlot(hotbarTexId);
        const float hotbarW = static_cast<float>(hotbarSlot.width) * 2.f;
        const float hotbarH = static_cast<float>(hotbarSlot.height) * 2.f;
        const float x = vpX / 2.f - hotbarW * 0.5f;
        const float y = vpY - hotbarH;
        this->hotbar = dynamic_cast<ImageWidget*>(this->root->addChild(std::make_unique<ImageWidget>(
            this->textureRegistry, "hotbar", glm::vec2{x, y}, glm::vec2{hotbarW, hotbarH}
        )));

        // Hotbar selection
        {
            const auto texId = this->textureRegistry.getByName("hotbar_selection");
            const auto& slot = this->textureRegistry.getSlot(texId);
            const float w = static_cast<float>(slot.width) * 2.f;
            const float h = static_cast<float>(slot.height) * 2.f;
            this->hotbarSelection = dynamic_cast<ImageWidget*>(
                this->hotbar->addChild(std::make_unique<ImageWidget>(
                    this->textureRegistry, "hotbar_selection", glm::vec2{-3.5f, -3.5f}, glm::vec2{w, h}
                ))
            );
        }

        // Items
        {
            constexpr auto itemTexSize = glm::vec2{64.f};
            for (int i = 0; i < 9; i++) {
                constexpr float invSlotW = 80.f;
                const float slotX = 1.f + (invSlotW * static_cast<float>(i) + (invSlotW * 0.5f) - (itemTexSize.x * 0.5f));
                const float slotY = (hotbarH * 0.5f) - (itemTexSize.y * 0.5f);

                auto icon = std::make_unique<ImageWidget>(
                    this->textureRegistry, TextureRegistry::MISSING, glm::vec2{slotX, slotY}, itemTexSize
                );

                icon->bind([this, i]() -> TextureId {
                    const auto& stack = this->hotbarInventory.items[i];
                    const auto& itemMeta = this->itemRegistry.get(stack.itemId);
                    return itemMeta.getIcon();
                });

                icon->bindVisibility([this, i]() -> bool {
                    return this->hotbarInventory.items[i].stackSize > 0;
                });

                icon->setVisible(false);

                // Stack Size text
                {
                    const auto stackSizeText = dynamic_cast<TextWidget*>(icon->addChild(
                        std::make_unique<TextWidget>(this->font, glm::vec2{itemTexSize.x + 6.f, itemTexSize.y - 28.f}, 0.7f)
                    ));

                    stackSizeText->setAlignment(TextAlign::Right);

                    stackSizeText->bind([this, i]() -> std::string {
                        return std::to_string(this->hotbarInventory.items[i].stackSize);
                    });

                    stackSizeText->bindVisibility([this, i]() -> bool {
                        return this->hotbarInventory.items[i].stackSize > 1;
                    });
                }

                this->hotbar->addChild(std::move(icon));
            }
        }
    }

    // Track viewport size for resize handling
    this->cachedViewportSize = vpSize;

    // Initial build
    this->rebuildVertexBuffer();
}

glm::mat4 GUIPanel::getGUIProjectionMatrix() const
{
    const auto& vpSize = this->viewport.getSize();

    return glm::ortho(
        0.0f, static_cast<float>(vpSize.x),
        static_cast<float>(vpSize.y), 0.0f,
        -1.f, 1.0f
    );
}

void GUIPanel::rebuildVertexBuffer()
{
    // Build widgets
    this->vertexBuffer.clear();
    this->root->build(this->vertexBuffer, {0.f, 0.f});
    this->root->clearDirty();
    this->vao.bind();
    this->vao.storeGuiData(this->vertexBuffer);
    this->vao.unbind();

    // Build MSDF
    this->fontBuffer.clear();
    this->root->buildMsdfTree(this->fontBuffer, {0.f, 0.f});
    this->fontVao.bind();
    this->fontVao.storeMsdfData(this->fontBuffer);
    this->fontVao.unbind();
}

void GUIPanel::update(const glm::vec3 &pos, const glm::vec3 &forward, const ECS::Hotbar& hotbarInv)
{
    this->currentPos = pos;
    this->currentForward = forward;
    this->hotbarInventory = hotbarInv;

    // Check for viewport resize
    const auto currentVp = this->viewport.getSize();
    if (currentVp != this->cachedViewportSize) {
        this->cachedViewportSize = currentVp;
        this->onViewportResize(currentVp);
    }

    // Evaluates bindings
    this->root->tick();

    // Rebuild VBO only if something changed
    if (this->root->isAnyDirty())
        this->rebuildVertexBuffer();
}

void GUIPanel::render()
{
    const auto& projMatrix = this->getGUIProjectionMatrix();

    this->shader.use();
    this->shader.setProjectionMatrix(projMatrix);
    this->vao.draw();

    if (!this->fontBuffer.empty()) {
        this->fontShader.use();
        this->fontShader.setProjectionMatrix(projMatrix);
        this->font.bindTexture();
        this->fontVao.draw();
    }
}


// Events
void GUIPanel::onViewportResize(const glm::ivec2 newSize) const
{
    this->crosshair->setVertices(getCrosshairVertices(newSize));

    // Hotbar: centered horizontally, anchored to bottom
    {
        const auto vpX = static_cast<float>(newSize.x);
        const auto vpY = static_cast<float>(newSize.y);
        const auto texId = this->textureRegistry.getByName("hotbar");
        const auto& slot = this->textureRegistry.getSlot(texId);
        const float w = static_cast<float>(slot.width) * 1.5f;
        const float h = static_cast<float>(slot.height) * 1.5f;
        this->hotbar->setPosition({vpX / 2.f - w * 0.5f, vpY - h - 20.f});
    }
}

void GUIPanel::onHotbarSlotChanged(const int slot) const
{
    // TODO: Make responsive with viewport size

    if (!this->hotbarSelection)
        return;

    const float x = 80.f * static_cast<float>(slot) - 3.5f;
    const float y = this->hotbarSelection->getPosition().y;

    this->hotbarSelection->setPosition({x, y});
}

void GUIPanel::toggleDebugPanel() const
{
    this->debugPanel->setVisible(!this->debugPanel->isVisible());
}
