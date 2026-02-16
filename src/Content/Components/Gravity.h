#ifndef FARFIELD_GRAVITY_H
#define FARFIELD_GRAVITY_H

namespace ECS {
    struct Gravity {
        float strength = 0.008f;
        float terminalVelocity = -(strength * 40);
    };
}

#endif