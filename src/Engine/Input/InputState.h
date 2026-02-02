#ifndef RE_MINECRAFT_INPUTSTATE_H
#define RE_MINECRAFT_INPUTSTATE_H

#include <utility>
#include <algorithm>

#include <GLFW/glfw3.h>

namespace Inputs
{
    enum class Keys : uint16_t
    {
        SPACE = GLFW_KEY_SPACE,
        W = GLFW_KEY_W,
        S = GLFW_KEY_S,
        A = GLFW_KEY_A,
        D = GLFW_KEY_D,
        Q = GLFW_KEY_Q,
        E = GLFW_KEY_E,
        P = GLFW_KEY_P
    };

    enum class Mouse : uint8_t
    {
        LEFT = GLFW_MOUSE_BUTTON_LEFT,
        RIGHT = GLFW_MOUSE_BUTTON_RIGHT,
        MIDDLE = GLFW_MOUSE_BUTTON_MIDDLE
    };

    enum class Scroll : signed char
    {
        NONE = 0,
        UP = 1,
        DOWN = -1
    };
}

struct InputState
{
    bool keyDown[GLFW_KEY_LAST + 1]{};
    bool keyPressed[GLFW_KEY_LAST + 1]{};
    bool keyReleased[GLFW_KEY_LAST + 1]{};

    bool mouseDown[GLFW_MOUSE_BUTTON_LAST + 1]{};
    bool mousePressed[GLFW_MOUSE_BUTTON_LAST + 1]{};
    bool mouseReleased[GLFW_MOUSE_BUTTON_LAST + 1]{};

    double mouseX{};
    double mouseY{};
    Inputs::Scroll scroll{Inputs::Scroll::NONE};

    [[nodiscard]] bool isKeyDown(const Inputs::Keys key) const { return keyDown[std::to_underlying(key)]; };
    [[nodiscard]] bool isKeyPressed(const Inputs::Keys key) const { return keyPressed[std::to_underlying(key)]; };
    [[nodiscard]] bool isKeyReleased(const Inputs::Keys key) const { return keyReleased[std::to_underlying(key)]; };

    [[nodiscard]] bool isMouseButtonDown(const Inputs::Mouse button) const { return mouseDown[std::to_underlying(button)]; };
    [[nodiscard]] bool isMouseButtonPressed(const Inputs::Mouse button) const { return mousePressed[std::to_underlying(button)]; };
    [[nodiscard]] bool isMouseButtonReleased(const Inputs::Mouse button) const { return mouseReleased[std::to_underlying(button)]; };

    void clear()
    {
        std::fill_n(keyPressed, sizeof(keyPressed), false);
        std::fill_n(keyReleased, sizeof(keyReleased), false);
        std::fill_n(mousePressed, sizeof(mousePressed), false);
        std::fill_n(mouseReleased, sizeof(mouseReleased), false);
        this->scroll = Inputs::Scroll::NONE;
    }

    static void keyInputCallback(GLFWwindow* window, const int key, [[maybe_unused]] const int scancode, const int action, [[maybe_unused]] const int mods)
    {
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
    static void mouseButtonInputCallback(GLFWwindow* window, const int button, const int action, [[maybe_unused]] const int mods)
    {
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
    static void mouseInputCallback(GLFWwindow* window, const double x, const double y)
    {
        const auto glfwPointer = glfwGetWindowUserPointer(window);

        if (glfwPointer == nullptr)
            return;

        auto* input = static_cast<InputState*>(glfwPointer);

        input->mouseX = x;
        input->mouseY = y;
    }
    static void mouseScrollInputCallback(GLFWwindow* window, [[maybe_unused]] double xoffset, const double yoffset)
    {
        const auto glfwPointer = glfwGetWindowUserPointer(window);

        if (glfwPointer == nullptr)
            return;

        auto* input = static_cast<InputState*>(glfwPointer);

        input->scroll = yoffset < 0 ? Inputs::Scroll::DOWN : Inputs::Scroll::UP;
    }
};

#endif //RE_MINECRAFT_INPUTSTATE_H