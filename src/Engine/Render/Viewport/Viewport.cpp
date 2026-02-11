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

void Viewport::initViewport()
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

    // Create MSAA framebuffer
    fbWidth = viewportSize.x;
    fbHeight = viewportSize.y;
    createMSAABuffers();
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

void Viewport::closeWindow()
{
    deleteMSAABuffers();

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

void Viewport::setCursorVisibility(const bool showCursor) const
{
    glfwSetInputMode(this->window, GLFW_CURSOR, showCursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
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
