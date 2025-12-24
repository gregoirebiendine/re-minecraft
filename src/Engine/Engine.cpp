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
    glfwSetWindowPos(window, (2048 / 2) - (this->W / 2),  (1280 / 2) - (this->H / 2)); // Should get the monitor size

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

    // Create all members
    this->shaders = std::make_unique<Shader>();
    this->world = std::make_unique<World>();
    this->camera = std::make_unique<Camera>(glm::vec3{0.0f, 16.0f, 18.0f});
    this->atlas = std::make_unique<Atlas>();

    if (!this->shaders || !this->world || !this->camera || !this->atlas)
        throw std::runtime_error("Failed to initialize shader program");
}

Engine::~Engine() {
    glfwDestroyWindow(this->window);
    glfwTerminate();
}


void Engine::loop() {
    while (!glfwWindowShouldClose(this->window)) {
        this->camera->handleInputs(this->window);
        this->render();
        glfwPollEvents();
    }
}

void Engine::render() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    this->camera->applyMatrix(45.0f, this->shaders, 1); // Should be width/height
    this->atlas->bind();
    this->world->render();

    glfwSwapBuffers(this->window);
}