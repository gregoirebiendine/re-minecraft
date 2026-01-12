#version 460 core

layout (location = 0) in uvec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in vec3 normal;

uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 ModelMatrix;

out vec2 texCoord;
out vec3 currentPos;
out vec3 currentNormal;

void main()
{
    currentPos = vec3(ModelMatrix * vec4(pos, 1.0f));
    texCoord = tex;
    currentNormal = normal;
    gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(pos, 1.0);
}