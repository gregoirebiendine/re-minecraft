#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;

out vec2 texCoord;

uniform mat4 ViewMatrix;

void main()
{
    vec3 newPos = vec3(pos.x + (gl_InstanceID*2), pos.y, pos.z);
    gl_Position = ViewMatrix * vec4(newPos, 1.0);
    texCoord = tex;
}