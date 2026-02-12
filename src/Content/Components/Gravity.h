#ifndef FARFIELD_GRAVITY_H
#define FARFIELD_GRAVITY_H

namespace ECS {
    struct Gravity {
        float strength = 0.01f;
        float terminalVelocity = -0.22f;
    };
}

#endif