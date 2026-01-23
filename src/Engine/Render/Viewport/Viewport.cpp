#include "Viewport.h"

void Viewport::initGLFW()
{
    if (!glfwInit())
        throw std::runtime_error("Cannot initialize GLFW3");

    // GLFW Hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Use MSAA 4x
    glfwWindowHint(GLFW_SAMPLES, 4);
}

void Viewport::initWindow(InputState* inputs)
{
    const auto viewportSize = this->settings.getViewportSize();

    initGLFW();

    this->window = glfwCreateWindow(viewportSize.x, viewportSize.y, "Farfield", nullptr, nullptr);
    if (!this->window) {
        glfwTerminate();
        throw std::runtime_error("Failed to open GLFW window");
    }

    glfwMakeContextCurrent(this->window);

    // Center window
    const auto videoMode = getVideoMode();
    glfwSetWindowPos(this->window, (videoMode->width / 2) - (viewportSize.x / 2),  (videoMode->height / 2) - (viewportSize.y / 2));

    // VSync
    glfwSwapInterval(this->settings.isVSync());

    // Enable RAW mouse input
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(this->window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    // Forward input state array to GLFW
    glfwSetWindowUserPointer(this->window, inputs);

    // Register inputs callback functions to GLFW
    glfwSetKeyCallback(this->window, InputState::keyInputCallback);
    glfwSetCursorPosCallback(this->window, InputState::mouseInputCallback);
    glfwSetMouseButtonCallback(this->window, InputState::mouseButtonInputCallback);
    glfwSetScrollCallback(this->window, InputState::mouseScrollInputCallback);

    // Initialize GLAD Manager
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        throw std::runtime_error("Cannot initialize GLAD");

    // Setup STBI image load
    stbi_set_flip_vertically_on_load(true);

    // Setup ImGui implementation
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(this->window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    // Set window aspect ratio
    this->aspectRatio = static_cast<float>(viewportSize.x) / static_cast<float>(viewportSize.y);
}

void Viewport::initViewport() const
{
    const auto viewportSize = this->settings.getViewportSize();

    // Create viewport
    glViewport(0, 0, viewportSize.x, viewportSize.y);

    // Change transparency function
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable 3D depth
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);

    // Enable culling
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    // Enable MSAA
    glEnable(GL_MULTISAMPLE);
}

void Viewport::closeWindow() const
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(this->window);
    glfwTerminate();
}

GLFWwindow* Viewport::getWindow() const
{
    return this->window;
}

bool Viewport::shouldClose() const
{
    return glfwWindowShouldClose(this->window);
}

void Viewport::pollEvents()
{
    glfwPollEvents();
}

void Viewport::swapBuffers() const
{
    glfwSwapBuffers(this->window);
}

Settings& Viewport::getSettings()
{
    return this->settings;
}

void Viewport::setVSyncUsage(const bool useVSync)
{
    this->settings.setVSync(useVSync);
    glfwSwapInterval(useVSync);
}

bool Viewport::useVSync() const
{
    return this->settings.isVSync();
}

float Viewport::getAspectRatio() const
{
    return this->aspectRatio;
}

void Viewport::toggleCursor(const bool isMouseCaptured) const
{
    glfwSetInputMode(this->window, GLFW_CURSOR, isMouseCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

// Statics
GLFWmonitor* Viewport::getMonitor()
{
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();

    if (!monitor) {
        glfwTerminate();
        throw std::runtime_error("Failed to get main monitor");
    }
    return monitor;
}

const GLFWvidmode* Viewport::getVideoMode()
{
    const GLFWvidmode* videoMode = glfwGetVideoMode(getMonitor());

    if (!videoMode) {
        glfwTerminate();
        throw std::runtime_error("Failed to get video mode");
    }
    return videoMode;
}
