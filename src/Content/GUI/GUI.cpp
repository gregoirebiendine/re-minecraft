#include "GUI.h"

GUI::GUI(const float windowRatio)
{
    this->shader = std::make_unique<Shader>(
    "../resources/shaders/UIShader/UIVertexShader.vert",
    "../resources/shaders/UIShader/UIFragShader.frag"
    );

    if (!this->shader)
        throw std::runtime_error("GUI Shader creation failed");

    const float VSIZE = (SIZE * windowRatio);
    const float VTHICKNESS = (THICKNESS / windowRatio);

    const std::vector<GLfloat> vertices = {
        // Horizontal first triangle
        -SIZE, -THICKNESS,
        -SIZE, THICKNESS,
        SIZE, -THICKNESS,
        // Horizontal second triangle
        SIZE, -THICKNESS,
        -SIZE, THICKNESS,
        SIZE, THICKNESS,

        // Vertical first triangle
        -VTHICKNESS, VSIZE,
        VTHICKNESS, VSIZE,
        -VTHICKNESS, -VSIZE,
        // Vertical second triangle
        VTHICKNESS, VSIZE,
        VTHICKNESS, -VSIZE,
        -VTHICKNESS, -VSIZE,
    };

    this->CrosshairVAO.bind();
    this->CrosshairVAO.addData<GLfloat, GL_FLOAT>(vertices, 0, 2);
    this->CrosshairVAO.unbind();
}

void GUI::renderCrosshair() const
{
    glDisable(GL_DEPTH_TEST);

    this->shader->use();

    this->CrosshairVAO.bind();
    glDrawArrays(GL_TRIANGLES, 0, 24);
    this->CrosshairVAO.unbind();

    glEnable(GL_DEPTH_TEST);
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
