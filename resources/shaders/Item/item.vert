#version 460 core

struct TextureSlot {
    float u0, v0, u1, v1;
    uint layer;
    uint _pad0, _pad1, _pad2;
};

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uvs;

layout (std430, binding = 0) readonly buffer TextureSlots {
    TextureSlot slots[];
};

out vec2 currentUvs;
out vec4 atlasUvBounds;
out float shade;
flat out uint currentLayer;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform uint LayerId;

void main()
{
    TextureSlot slot = slots[LayerId];
    currentUvs = uvs;
    atlasUvBounds = vec4(slot.u0, slot.v0, slot.u1, slot.v1);
    currentLayer = slot.layer;

    vec3 n = normalize(normal);
    if ( n.y >  0.5)
        shade = 1.0;   // top
    else if ( n.y < -0.5)
        shade = 0.5;   // bottom
    else if (abs(n.x) > 0.5)
        shade = 0.8; // left / right sides
    else
        shade = 0.9;   // front / back (z faces)

    gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(pos, 1.0);
}