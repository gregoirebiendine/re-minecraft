#version 460 core

layout (location = 0) in uvec3 pos;
layout (location = 1) in vec2 tex;

out vec2 TexCoord;

uniform mat4 ViewMatrix;

void main()
{
    TexCoord = tex;
    gl_Position = ViewMatrix * vec4(pos, 1.0);
}