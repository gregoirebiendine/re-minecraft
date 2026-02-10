#version 460 core

struct TextureSlot {
    float u0, v0, u1, v1;
    uint layer;
    uint _pad0, _pad1, _pad2;
};

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec2 uvs;
layout (location = 3) in uint texId;

layout (std430, binding = 0) readonly buffer TextureSlots {
    TextureSlot slots[];
};

out vec2 currentUvs;
out vec4 atlasUvBounds;
flat out uint currentLayer;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

void main()
{
    // Look up texture slot from atlas
    TextureSlot slot = slots[texId];

    // Output
    currentUvs = uvs;
    atlasUvBounds = vec4(slot.u0, slot.v0, slot.u1, slot.v1);
    currentLayer = slot.layer;

    // Position
    gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(pos, 1.0);
}