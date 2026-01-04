#ifndef RE_MINECRAFT_INPUTSTATE_H
#define RE_MINECRAFT_INPUTSTATE_H

#include <GLFW/glfw3.h>

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
};

#endif //RE_MINECRAFT_INPUTSTATE_H