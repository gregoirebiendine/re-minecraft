#version 460 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 color;

uniform mat4 ProjectionMatrix;

out vec2 currentUv;
out vec4 currentColor;

void main()
{
    currentUv = uv;
    currentColor = color;
    gl_Position = ProjectionMatrix * vec4(pos, 0.0, 1.0);
}