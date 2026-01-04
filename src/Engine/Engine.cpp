#include "Engine.h"

Engine::Engine()
{
    if (!glfwInit())
        throw std::runtime_error("Cannot initialize GLFW3");

    // Pass GLFW version to the lib
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create the window
    this->window = glfwCreateWindow(WindowSize.x, WindowSize.y, "Re Minecraft", nullptr, nullptr);
    if (!this->window) {
        glfwTerminate();
        throw std::runtime_error("Failed to open window");
    }

    // Get main monitor and get monitor screen size
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if (!monitor)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to get main monitor");
    }

    const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
    if (!videoMode)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to get video mode");
    }

    this->ScreenSize = glm::ivec2(videoMode->width, videoMode->height);

    // Center window
    glfwSetWindowPos(window, (videoMode->width / 2) - (WindowSize.x / 2),  (videoMode->height / 2) - (WindowSize.y / 2));

    // Make window current context for GLFW
    glfwMakeContextCurrent(this->window);

    // Initialize GLAD Manager
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        throw std::runtime_error("Cannot initialize GLAD");

    // Create viewport
    glViewport(0, 0, WindowSize.x, WindowSize.y);

    // Enable 3D depth
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Setup STBI image load
    stbi_set_flip_vertically_on_load(true);

    // Setup ImGui implementation
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(this->window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    // Enable RAW mouse input
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    // Forward input state array to GLFW
    glfwSetWindowUserPointer(this->window, &this->inputs);

    // Register inputs callback functions to GLFW
    glfwSetKeyCallback(window, keyInputCallback);
    glfwSetCursorPosCallback(window, mouseInputCallback);
    glfwSetMouseButtonCallback(window, mouseButtonInputCallback);

    // Create all members
    this->shaders = std::make_unique<Shader>();
    this->atlas = std::make_unique<Atlas>();
    this->camera = std::make_unique<Camera>(glm::vec3{16.0f, 26.0f, 35.0f});
    this->world = std::make_unique<World>();

    if (!this->shaders || !this->world || !this->camera || !this->atlas)
        throw std::runtime_error("Failed to initialize pointers");
}

Engine::~Engine() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(this->window);
    glfwTerminate();
}

void Engine::loop()
{
    while (!glfwWindowShouldClose(this->window)) {
        glfwPollEvents();
        this->handleInputs();
        this->render();
        this->clearInputs();
    }
}

void Engine::handleInputs() const
{
    this->camera->moveCamera(this->inputs.mouseX, this->inputs.mouseY);

    if (this->inputs.mousePressed[GLFW_MOUSE_BUTTON_LEFT])
    {
        if (const Raycast::Hit raycast = this->camera->raycast(*this->world); raycast.hit)
            this->world->setBlock(raycast.pos.x, raycast.pos.y, raycast.pos.z, Material::AIR);
    }

    if (this->inputs.mousePressed[GLFW_MOUSE_BUTTON_RIGHT])
    {
        if (const Raycast::Hit raycast = this->camera->raycast(*this->world); raycast.hit)
            this->world->setBlock(raycast.previousPos.x, raycast.previousPos.y, raycast.previousPos.z, Material::GRASS);
    }

    if (this->inputs.keyPressed[GLFW_KEY_SPACE])
    {
        glfwSetInputMode(window, GLFW_CURSOR, !this->camera->getMouseCapture() ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        this->camera->toggleMouseCapture();
    }

    if (this->inputs.keyDown[GLFW_KEY_W])
        this->camera->move({0,0,1});
    if (this->inputs.keyDown[GLFW_KEY_S])
        this->camera->move({0,0,-1});
    if (this->inputs.keyDown[GLFW_KEY_A])
        this->camera->move({-1,0,0});
    if (this->inputs.keyDown[GLFW_KEY_D])
        this->camera->move({1,0,0});
    if (this->inputs.keyDown[GLFW_KEY_Q])
        this->camera->move({0,1,0});
    if (this->inputs.keyDown[GLFW_KEY_E])
        this->camera->move({0,-1,0});
}

void Engine::clearInputs()
{
    std::fill_n(this->inputs.keyPressed, sizeof(this->inputs.keyPressed), false);
    std::fill_n(this->inputs.keyReleased, sizeof(this->inputs.keyReleased), false);
    std::fill_n(this->inputs.mousePressed, sizeof(this->inputs.mousePressed), false);
    std::fill_n(this->inputs.mouseReleased, sizeof(this->inputs.mouseReleased), false);
}

void Engine::render() const
{
    // Clear window and buffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Create ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Apply camera position and rotation
    this->setViewMatrix();

    // Render World (chunks)
    this->atlas->bind();
    this->world->render(*this->shaders);

    // Get Camera state for GUI
    const auto cameraPos = this->camera->getPosition();
    const auto cameraRotation = this->camera->getRotation();

    ImGui::Begin("Debug");
    ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
    ImGui::Text("X: %.2f, Y: %.2f, Z: %.2f", cameraPos.x, cameraPos.y, cameraPos.z);
    ImGui::Text("Yaw: %.2f, Pitch: %.2f", cameraRotation.x, cameraRotation.y);
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update buffer
    glfwSwapBuffers(this->window);
}

void Engine::setViewMatrix() const
{
    const auto forward = this->camera->getForwardVector();
    const auto cameraPos = this->camera->getPosition();
    const glm::mat4 view = glm::lookAt(cameraPos, cameraPos + forward, {0,1,0});
    const glm::mat4 projection = glm::perspective(Camera::FOV, static_cast<float>(WindowSize.x)/static_cast<float>(WindowSize.y), 0.1f, 100.f);

    this->shaders->setUniformMat4("ViewMatrix", projection * view);
}

// Statics callback
void keyInputCallback(GLFWwindow* window, const int key, const int scancode, const int action, const int mods)
{
    UNUSED(scancode);
    UNUSED(mods);

    const auto glfwPointer = glfwGetWindowUserPointer(window);

    if (glfwPointer == nullptr || key < 0 || key > GLFW_KEY_LAST)
        return;

    auto* input = static_cast<InputState*>(glfwPointer);

    if (action == GLFW_PRESS)
    {
        input->keyDown[key] = true;
        input->keyPressed[key] = true;
    }
    else if (action == GLFW_RELEASE)
    {
        input->keyDown[key] = false;
        input->keyReleased[key] = true;
    }
}

void mouseButtonInputCallback(GLFWwindow* window, const int button, const int action, const int mods)
{
    UNUSED(mods);

    const auto glfwPointer = glfwGetWindowUserPointer(window);

    if (glfwPointer == nullptr || button < 0 || button > GLFW_MOUSE_BUTTON_LAST)
        return;

    auto* input = static_cast<InputState*>(glfwPointer);

    if (action == GLFW_PRESS)
    {
        input->mouseDown[button] = true;
        input->mousePressed[button] = true;
    }
    else if (action == GLFW_RELEASE)
    {
        input->mouseDown[button] = false;
        input->mouseReleased[button] = true;
    }
}

void mouseInputCallback(GLFWwindow* window, const double x, const double y)
{
    const auto glfwPointer = glfwGetWindowUserPointer(window);

    if (glfwPointer == nullptr)
        return;

    auto* input = static_cast<InputState*>(glfwPointer);

    input->mouseX = x;
    input->mouseY = y;
}
