#version 460 core

layout (location = 0) in uvec3 pos;
layout (location = 1) in vec2 tex;

uniform mat4 ViewMatrix;

out vec2 TexCoord;

void main()
{
    TexCoord = tex;
    gl_Position = ViewMatrix * vec4(pos, 1.0);
}