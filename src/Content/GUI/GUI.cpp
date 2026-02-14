#include "GUI.h"
#include "ChunkPos.h"
#include "OutlineVertices.h"
#include "CrosshairVertices.h"

GUI::GUI(const Font& _font, const TextureRegistry& _textureRegistry, Settings& _settings) :
    font(_font),
    textureRegistry(_textureRegistry),
    settings(_settings),
    guiShader("/resources/shaders/UI/"),
    outlineShader("/resources/shaders/Outline/"),
    fontTexId(this->font.getTextureID())
{
    const auto& viewportSize = this->settings.getViewportSize();

    // Set shader uniforms
    this->guiShader.use();
    this->guiShader.setUniformInt("Textures", 0);

    // General GUI
    this->data = getCrosshairVertices(viewportSize);
    this->createRectangle(0, static_cast<float>(viewportSize.y) - 80, 400, 80, {20,20,20,0.6f});
    this->createText(20, 600, "Bonjour je m'appelle Grégoire Biendiné, j'ai 23ans et j'ai un grand cœur");
    this->createImage(static_cast<float>(viewportSize.x) / 2.f, static_cast<float>(viewportSize.y), "hotbar");
    this->createImage(static_cast<float>(viewportSize.x) / 2.f, static_cast<float>(viewportSize.y), "hotbar_selection");

    // Upload GUI data
    this->guiVao.bind();
    this->guiVao.storeGuiData(this->data);
    this->guiVao.unbind();

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

void GUI::createRectangle(const float x, const float y, const float width, const float height, const DigitalColor color)
{
    const float x1 = x + width;
    const float y1 = y + height;

    this->data.insert(this->data.end(), {
        // First triangle
        {{x, y}, {-1.f, -1.f}, color},
        {{x, y1}, {-1.f, -1.f}, color},
        {{x1, y1}, {-1.f, -1.f}, color},

        // Second triangle
        {{x, y}, {-1.f, -1.f}, color},
        {{x1, y1}, {-1.f, -1.f}, color},
        {{x1, y}, {-1.f, -1.f}, color},
    });
}

void GUI::createText(const float x, const float y, const std::string &text)
{
    constexpr float scale = 1.f;
    const auto& uvs = this->font.getUVFromString(text);
    const DigitalColor color{255, 255, 255, 1.f};
    const float y1 = y + Font::CHAR_SIZE_Y * scale;

    float curX = x;
    for (const auto& uv : uvs) {
        float charX1 = curX + Font::CHAR_SIZE_X * scale;
        this->data.insert(this->data.end(), {
            {{curX, y},    uv[0], color, this->fontTexId},
            {{curX, y1},   uv[1], color, this->fontTexId},
            {{charX1, y1}, uv[2], color, this->fontTexId},

            {{curX, y},    uv[3], color, this->fontTexId},
            {{charX1, y1}, uv[4], color, this->fontTexId},
            {{charX1, y},  uv[5], color, this->fontTexId},
        });
        curX += Font::CHAR_SIZE_X * scale;
    }
}

void GUI::createImage(float x, float y, const std::string &image)
{
    const auto textId = this->textureRegistry.getByName(image);
    const auto& textSlot = this->textureRegistry.getSlot(textId);
    const DigitalColor color{255, 255, 255, 1.f};

    const float w = static_cast<float>(textSlot.width) * 1.5f;
    const float h = static_cast<float>(textSlot.height) * 1.5f;

    x -= w * 0.5f;
    y -= h + 20;
    const float x1 = x + w;
    const float y1 = y + h;

    this->data.insert(this->data.end(), {
        // First triangle
        {{x, y}, {0.f, 0.f}, color, textId},
        {{x, y1}, {0.f, 1.f}, color, textId},
        {{x1, y1}, {1.f, 1.f}, color, textId},

        // Second triangle
        {{x, y}, {0.f, 0.f}, color, textId},
        {{x1, y1}, {1.f, 1.f}, color, textId},
        {{x1, y}, {1.f, 0.f}, color, textId},
    });
}

void GUI::render(const glm::vec3& pos, const glm::vec3& forward, const std::string& selectedBlockName)
{
    glDisable(GL_DEPTH_TEST);
    glEnable( GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    createImGuiFrame();
    renderImGuiFrame(pos, forward, selectedBlockName);

    this->guiShader.use();
    this->guiShader.setProjectionMatrix(this->getGUIProjectionMatrix());

    this->guiVao.bind();
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(this->data.size()));
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

void GUI::createImGuiFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUI::renderImGuiFrame(const glm::vec3 pos, const glm::vec3 forward, const std::string& selectedBlockName)
{
    const auto facing = DirectionUtils::forwardVectorToCardinal(forward);
    const ChunkPos cp{
        static_cast<int>(pos.x) / 16,
        static_cast<int>(pos.y) / 16,
        static_cast<int>(pos.z) / 16
    };

    ImGui::Begin("Debug");
    ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
    ImGui::Text("Position : %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);
    ImGui::Text("Chunk : %d, %d, %d", cp.x, cp.y, cp.z);
    ImGui::Text("Facing : %s", facing.c_str());
    ImGui::Text("Selected block : %s", selectedBlockName.c_str());
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// Statics
float GUI::toScreenSpace(const float v, const float minIn, const float maxIn)
{
    const float res = -1 + (1 - (-1)) * ((v - minIn) / (maxIn - minIn));
    return std::clamp(res, -1.0f, 1.0f);
}

float GUI::percent(const float baseValue, const float percentage)
{
    return baseValue * (percentage/100.0f);
}