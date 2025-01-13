#include "Engine.h"

Engine &Engine::Instance()
{
    static Engine instance;
    return instance;
}

void Engine::init()
{
    if (!glfwInit())
        throw std::runtime_error("Cannot initialize GLFW3");

    // Pass GLFW version to the lib
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create the window
    Engine::Instance().window = glfwCreateWindow(1000, 1000, "Re Minecraft", nullptr, nullptr);
    if (!Engine::Instance().window) {
        glfwTerminate();
        throw std::runtime_error("Failed to open windows");
    }
    glfwMakeContextCurrent(Engine::Instance().window);

    // Initialize GLAD Manager
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        throw std::runtime_error("Cannot initialize GLAD");

    // Create viewport
    glViewport(0, 0, 1000, 1000);

    // Enable 3D depth
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Setup STBI image load
    stbi_set_flip_vertically_on_load(true);

    // Create Drawer
    Engine::Instance().drawer = new Drawer();

    // Create Camera with position
    Engine::Instance().camera = new Camera({0.0f, 0.0f, 8.0f});
}

void Engine::loop() {
    while (!glfwWindowShouldClose(Engine::Instance().window)) {
        Engine::Instance().camera->handleInputs(Engine::Instance().window);
        Engine::draw();
        glfwPollEvents();
    }
}

void Engine::draw() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Engine::Instance().camera->applyMatrix(45.0f, Engine::Instance().drawer->getShader(), 1000/1000);
    Engine::Instance().drawer->draw();

    glfwSwapBuffers(Engine::Instance().window);
}

void Engine::destroy() {
    delete Engine::Instance().drawer;
    delete Engine::Instance().camera;
    glfwDestroyWindow(Engine::Instance().window);
    glfwTerminate();
}