#include "GUI.h"
#include "Engine.h"

GUI::GUI(const float windowRatio)
{
    this->shader = std::make_unique<Shader>(
    "../resources/shaders/UIShader/UIVertexShader.vert",
    "../resources/shaders/UIShader/UIFragShader.frag"
    );

    if (!this->shader)
        throw std::runtime_error("GUI Shader creation failed");

    const auto c = color(200,200,200, 0.8f);
    this->shader->use();
    this->shader->setUniformVec4("color", {c.r, c.g, c.b, c.a});

    this->createCrosshair();
    this->VAO.bind();
    this->VAO.addData<GLfloat, GL_FLOAT>(this->vertices, 0, 2);
    this->VAO.unbind();
}

void GUI::createCrosshair() {
    constexpr glm::vec2 mid = {Engine::WindowSize.x / 2 , Engine::WindowSize.y / 2};

    this->createRectangle(
        mid.x - (SIZE / 2),
        mid.y - (THICKNESS / 2),
        SIZE,
        THICKNESS,
        {0.f,0.f,0.f,1.f} // wrong notation
    );

    this->createRectangle(
        mid.x - (THICKNESS / 2),
        mid.y - (SIZE / 2),
        THICKNESS,
        SIZE,
        {0.f,0.f,0.f,1.f} // wrong notation
    );
}

void GUI::createRectangle(const float x, const float y, const float width, const float height, UNUSED const DigitalColor color)
{
    const float x1 = x + width;
    const float y1 = y + height;

    this->vertices.insert(this->vertices.end(), {
        // First triangle
        x, y,
        x1, y,
        x1, y1,
        // Second triangle
        x, y,
        x1, y1,
        x, y1,
    });
}

void GUI::render() const
{
    const glm::mat4 ProjectionMatrix = glm::ortho(
        0.0f, static_cast<float>(Engine::WindowSize.x),
        static_cast<float>(Engine::WindowSize.y), 0.0f,
        -1.f, 1.0f
    );

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    this->shader->use();
    this->shader->setUniformMat4("ProjectionMatrix", ProjectionMatrix);

    this->VAO.bind();
    glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(this->vertices.size()));
    this->VAO.unbind();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void GUI::createImGuiFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUI::renderImGuiFrame(const Camera& camera, const BlockRegistry& blockRegistry)
{
    const auto cameraPos = camera.getPosition();
    const auto cameraRotation = camera.getRotation();
    const auto selectedBlock = camera.getSelectedMaterial();

    ImGui::Begin("Debug");
    ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
    ImGui::Text("X: %.2f, Y: %.2f, Z: %.2f", cameraPos.x, cameraPos.y, cameraPos.z);
    ImGui::Text("Yaw: %.2f, Pitch: %.2f", cameraRotation.x, cameraRotation.y);
    ImGui::Text("Selected block : %s", blockRegistry.get(selectedBlock).getName().c_str());
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

float GUI::percent(const float baseValue, float percentage)
{
    return baseValue * (percentage/100.0f);
}

DigitalColor GUI::color(const uint8_t r, const uint8_t g, const uint8_t b, const float a)
{
    return {
        static_cast<float>(r) / 255.0f,
        static_cast<float>(g) / 255.0f,
        static_cast<float>(b) / 255.0f,
        a
    };
}
