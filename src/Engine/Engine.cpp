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
    this->window = glfwCreateWindow(this->W, this->H, "Re Minecraft", nullptr, nullptr);
    if (!this->window) {
        glfwTerminate();
        throw std::runtime_error("Failed to open window");
    }

    // Center window
    glfwSetWindowPos(window, (1920 / 2) - (this->W / 2),  (1080 / 2) - (this->H / 2)); // Should get the monitor size

    // Make window current context for GLFW
    glfwMakeContextCurrent(this->window);

    // Initialize GLAD Manager
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        throw std::runtime_error("Cannot initialize GLAD");

    // Create viewport
    glViewport(0, 0, this->W, this->H);

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

    // Register input callback function to GLFW
    glfwSetKeyCallback(window, keyInputCallback);

    // Register mouse callback function to GLFW
    glfwSetCursorPosCallback(window, mouseInputCallback);

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

        std::fill_n(this->inputs.pressed, sizeof(this->inputs.pressed), false);
        std::fill_n(this->inputs.released, sizeof(this->inputs.released), false);
    }
}

void Engine::handleInputs() const
{
    this->camera->moveCamera(this->inputs.mouseX, this->inputs.mouseY);

    if (this->inputs.pressed[GLFW_KEY_SPACE])
    {
        glfwSetInputMode(window, GLFW_CURSOR, !this->camera->getMouseCapture() ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        this->camera->toggleMouseCapture();
    }

    if (this->inputs.down[GLFW_KEY_W])
        this->camera->move({0,0,1});
    if (this->inputs.down[GLFW_KEY_S])
        this->camera->move({0,0,-1});
    if (this->inputs.down[GLFW_KEY_A])
        this->camera->move({-1,0,0});
    if (this->inputs.down[GLFW_KEY_D])
        this->camera->move({1,0,0});
    if (this->inputs.down[GLFW_KEY_Q])
        this->camera->move({0,1,0});
    if (this->inputs.down[GLFW_KEY_E])
        this->camera->move({0,-1,0});
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
    this->camera->applyMatrix(90.0f, this->shaders, static_cast<float>(this->W)/static_cast<float>(this->H));

    // Render World (chunks)
    this->atlas->bind();
    this->world->render(*this->shaders);

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

void keyInputCallback(GLFWwindow* window, const int key, const int scancode, const int action, const int mods)
{
    const auto glfwPointer = glfwGetWindowUserPointer(window);

    if (glfwPointer == nullptr || key < 0 || key > GLFW_KEY_LAST)
        return;

    auto* input = static_cast<InputState*>(glfwPointer);

    if (action == GLFW_PRESS)
    {
        input->down[key] = true;
        input->pressed[key] = true;
    }
    else if (action == GLFW_RELEASE)
    {
        input->down[key] = false;
        input->released[key] = true;
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
