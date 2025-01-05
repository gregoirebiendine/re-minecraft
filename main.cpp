#include <Engine.h>

int main() {
    Engine::Instance().init(),
    Engine::Instance().loop();
    Engine::Instance().destroy();
    return 0;
}