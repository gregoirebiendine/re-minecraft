#version 460 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec4 color;

uniform mat4 ProjectionMatrix;

out vec4 OutColor;

void main()
{
    OutColor = color;
    gl_Position = ProjectionMatrix * vec4(pos.xy, 0.0, 1.0);
}