#version 460 core

struct TextureSlot {
    float u0, v0, u1, v1;
    uint layer;
    uint _pad0, _pad1, _pad2;
};

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 uvs;
layout (location = 2) in vec4 color;
layout (location = 3) in int texId;

layout (std430, binding = 0) readonly buffer TextureSlots {
    TextureSlot slots[];
};

uniform mat4 ProjectionMatrix;

out vec2 currentUvs;
out vec4 atlasUvBounds;
out vec4 currentColor;
flat out uint currentLayer;

void main()
{
    // Look up texture slot from atlas
    TextureSlot slot = slots[texId];

    // Output
    currentUvs = uvs;
    atlasUvBounds = vec4(slot.u0, slot.v0, slot.u1, slot.v1);
    currentColor = color;
    currentLayer = slot.layer;

    gl_Position = ProjectionMatrix * vec4(pos.xy, 0.0, 1.0);
}