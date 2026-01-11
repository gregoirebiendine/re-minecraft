#version 460 core

layout (location = 0) in uvec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in vec3 normal;

uniform mat4 ViewMatrix;
uniform mat4 ViewModel;

out vec2 texCoord;
out vec3 currentPos;
out vec3 currentNormal;

void main()
{
    texCoord = tex;
    currentPos = vec3(ViewModel * vec4(pos, 1.0f));
    currentNormal = normal;
    gl_Position = ViewMatrix * ViewModel * vec4(pos, 1.0);
}