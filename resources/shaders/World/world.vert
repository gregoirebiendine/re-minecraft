#version 460 core

layout (location = 0) in uint data1;
layout (location = 1) in uint data2;

// TextureSlot from the atlas packer
struct TextureSlot {
    float u0, v0, u1, v1;
    uint layer;
    uint _pad0, _pad1, _pad2;
};

layout (std430, binding = 0) readonly buffer TextureSlots {
    TextureSlot slots[];
};

uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 ModelMatrix;

out vec3 currentPos;
out vec3 currentNormal;
out vec2 currentUvs;
out vec4 atlasUvBounds;
flat out uint currentLayer;
flat out uint currentRotation;

// Normal lookup table
const vec3 NORMALS[6] = vec3[6](
    vec3( 0.0,  0.0, -1.0),  // NORTH
    vec3( 0.0,  0.0,  1.0),  // SOUTH
    vec3(-1.0,  0.0,  0.0),  // WEST
    vec3( 1.0,  0.0,  0.0),  // EAST
    vec3( 0.0,  1.0,  0.0),  // UP
    vec3( 0.0, -1.0,  0.0)   // DOWN
);

void main()
{
    // Unpack : position + normal index + rotation + ambient occlusion
    uint x = data1 & 0x1Fu;
    uint y = (data1 >> 5) & 0x1Fu;
    uint z = (data1 >> 10) & 0x1Fu;
    uint normalIndex = (data1 >> 15) & 0x7u;
    uint rotation = (data1 >> 18) & 0x7u;
    uint ao = (data1 >> 28) & 0xFu;

    // Unpack : uv + texId
    uint u = data2 & 0x1Fu;
    uint v = (data2 >> 5) & 0x1Fu;
    uint texId = (data2 >> 10) & 0xFFFFu;

    // Look up texture slot from atlas
    TextureSlot slot = slots[texId];

    // Build position
    vec3 localPos = vec3(float(x), float(y), float(z));
    vec4 worldPos = ModelMatrix * vec4(localPos, 1.0);

    // Output
    currentPos = worldPos.xyz;
    currentNormal = NORMALS[normalIndex];
    currentUvs = vec2(float(u), float(v));
    atlasUvBounds = vec4(slot.u0, slot.v0, slot.u1, slot.v1);
    currentLayer = slot.layer;
    currentRotation = rotation;

    gl_Position = ProjectionMatrix * ViewMatrix * worldPos;
}