#include "Viewport.h"

Viewport::Viewport(Settings &_settings, InputState* inputs) :
    settings(_settings)
{
    initGLFW();
    this->initWindow(inputs);
    this->initViewport();
}

void Viewport::initGLFW()
{
    if (!glfwInit())
        throw std::runtime_error("Cannot initialize GLFW3");

    // GLFW Hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // MSAA Samples
    glfwWindowHint(GLFW_SAMPLES, 4);
}

void Viewport::initWindow(InputState* inputs)
{
    this->window = glfwCreateWindow(this->size.x, this->size.y, "Farfield", nullptr, nullptr);
    if (!this->window) {
        glfwTerminate();
        throw std::runtime_error("Failed to open GLFW window");
    }

    glfwMakeContextCurrent(this->window);

    // Auto resize window
    const auto* videoMode = getVideoMode();
    const glm::ivec2 autoSize = getClosestResolution(videoMode);

    this->setSize(autoSize, false);
    glfwSetWindowSize(this->window, autoSize.x, autoSize.y);

    // Center window
    this->centerWindow(videoMode);

    // Set fps target to screen refresh rate
    this->settings.setFpsTarget(videoMode->refreshRate);

    // Enable VSync
    this->useVSync(true);

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
    glfwSetFramebufferSizeCallback(this->window, InputState::viewportResizeCallback);

    // Initialize GLAD Manager
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        throw std::runtime_error("Cannot initialize GLAD");
}

void Viewport::initViewport() const
{
    // Create viewport
    glViewport(0, 0, this->size.x, this->size.y);

    // Change transparency function
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable 3D depth
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);

    // Enable culling
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    // Disable integrated MSAA (will be used only for block outlines)
    glDisable(GL_MULTISAMPLE);
}

void Viewport::closeWindow() const
{
    glfwDestroyWindow(this->window);
    glfwTerminate();
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

void Viewport::centerWindow(const GLFWvidmode* videoMode) const
{
    glfwSetWindowPos(this->window, (videoMode->width / 2) - (this->size.x / 2),  (videoMode->height / 2) - (this->size.y / 2));
}

void Viewport::setSize(const glm::ivec2 _size, const bool modifyViewport)
{
    if (_size == glm::ivec2(0))
        return;

    this->lastSize = this->size;
    this->size = _size;
    this->setAspectRatio(static_cast<float>(_size.x) / static_cast<float>(_size.y));

    if (modifyViewport)
        glViewport(0, 0, _size.x, _size.y);
}

glm::ivec2 Viewport::getSize() const
{
    return this->size;
}

void Viewport::useVSync(const bool use) const
{
    this->settings.useVSync(use);
    glfwSwapInterval(use ? 1 : 0);
}

bool Viewport::isUsingVSync() const
{
    return this->settings.isUsingVSync();
}

void Viewport::setCursorVisibility(const bool showCursor) const
{
    glfwSetInputMode(this->window, GLFW_CURSOR, showCursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}


void Viewport::toggleFullscreen()
{
    const auto* videoMode = getVideoMode();
    const bool wouldBeFullscreen = !this->settings.isFullscreen();

    this->settings.setFullscreen(wouldBeFullscreen);

    if (wouldBeFullscreen) {
        GLFWmonitor *monitor = getMonitor();
        this->setSize({videoMode->width, videoMode->height});
        glfwSetWindowMonitor(this->window, monitor, 0, 0, videoMode->width, videoMode->height, videoMode->refreshRate);
    }
    else {
        glfwSetWindowMonitor(this->window, nullptr, 0, 0, this->lastSize.x, this->lastSize.y, videoMode->refreshRate);
        this->setSize(this->lastSize);
        this->centerWindow(videoMode);
    }
}

// Statics
GLFWmonitor* Viewport::getMonitor()
{
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();

    if (!monitor) {
        glfwTerminate();
        throw std::runtime_error("[Viewport::getMonitor] Failed to get main monitor");
    }
    return monitor;
}

const GLFWvidmode* Viewport::getVideoMode()
{
    const GLFWvidmode* videoMode = glfwGetVideoMode(getMonitor());

    if (!videoMode) {
        glfwTerminate();
        throw std::runtime_error("[Viewport::getVideoMode] Failed to get video mode");
    }
    return videoMode;
}

glm::ivec2 Viewport::getClosestResolution(const GLFWvidmode *videoMode)
{
    int closestHeight = 0;

    for (const auto& h : SCREEN_SIZES | std::views::keys) {
        if (h >= videoMode->height)
            continue;

        if (h >= closestHeight)
            closestHeight = h;
    }

    return {SCREEN_SIZES.at(closestHeight), closestHeight};
}