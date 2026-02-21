#include "Viewport.h"

Viewport::Viewport(Settings &_settings) :
    settings(_settings)
{}

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
    initGLFW();

    this->window = glfwCreateWindow(this->size.x, this->size.y, "Farfield", nullptr, nullptr);
    if (!this->window) {
        glfwTerminate();
        throw std::runtime_error("Failed to open GLFW window");
    }

    glfwMakeContextCurrent(this->window);

    // Center window
    const auto* videoMode = getVideoMode();
    this->centerWindow(videoMode);

    // Set fps target to screen refresh rate
    this->settings.setFpsTarget(videoMode->refreshRate);

    // VSync
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

    // Set window aspect ratio
    this->aspectRatio = static_cast<float>(this->size.x) / static_cast<float>(this->size.y);
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

    // Create MSAA framebuffer (should be removed because of weird texture artifacts)
    // fbWidth = viewportSize.x;
    // fbHeight = viewportSize.y;
    // createMSAABuffers();
}

void Viewport::createMSAABuffers()
{
    // Create multisampled FBO
    glGenFramebuffers(1, &msaaFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    // Create multisampled color buffer
    glGenRenderbuffers(1, &msaaColorBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, msaaColorBuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA_SAMPLES, GL_RGBA8, fbWidth, fbHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, msaaColorBuffer);

    // Create multisampled depth buffer
    glGenRenderbuffers(1, &msaaDepthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, msaaDepthBuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA_SAMPLES, GL_DEPTH24_STENCIL8, fbWidth, fbHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, msaaDepthBuffer);

    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error("MSAA Framebuffer is not complete");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Viewport::deleteMSAABuffers()
{
    if (msaaFBO) {
        glDeleteFramebuffers(1, &msaaFBO);
        msaaFBO = 0;
    }
    if (msaaColorBuffer) {
        glDeleteRenderbuffers(1, &msaaColorBuffer);
        msaaColorBuffer = 0;
    }
    if (msaaDepthBuffer) {
        glDeleteRenderbuffers(1, &msaaDepthBuffer);
        msaaDepthBuffer = 0;
    }
}

void Viewport::beginFrame() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
    glViewport(0, 0, fbWidth, fbHeight);
}

void Viewport::endFrame() const
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(
        0, 0, fbWidth, fbHeight,
        0, 0, fbWidth, fbHeight,
        GL_COLOR_BUFFER_BIT,
        GL_NEAREST
    );

    // Bind default framebuffer for any post-processing or UI
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Viewport::closeWindow() const
{
    // deleteMSAABuffers();

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

void Viewport::setSize(const glm::ivec2 _size)
{
    if (_size == glm::ivec2(0))
        return;

    this->size = _size;
    this->setAspectRatio(static_cast<float>(_size.x) / static_cast<float>(_size.y));

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

void Viewport::setAspectRatio(const float aspect)
{
    this->aspectRatio = aspect;
}

float Viewport::getAspectRatio() const
{
    return this->aspectRatio;
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
        glfwSetWindowMonitor(this->window, monitor, 0, 0, videoMode->width, videoMode->height, videoMode->refreshRate);
        this->setSize({videoMode->width, videoMode->height});
    }
    else {
        glfwSetWindowMonitor(this->window, nullptr, 0, 0, this->baseSize.x, this->baseSize.y, videoMode->refreshRate);
        this->setSize(this->baseSize);
        this->centerWindow(videoMode);
    }
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
