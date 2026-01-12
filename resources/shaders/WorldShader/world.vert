#version 460 core

layout (location = 0) in uvec3 pos;
layout (location = 1) in vec2 uvs;
layout (location = 2) in vec3 normal;
layout (location = 3) in uint texIndex;

uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 ModelMatrix;

out vec2 currentUvs;
out vec3 currentPos;
out vec3 currentNormal;
flat out uint currentTexIndex;

void main()
{
    currentPos = vec3(ModelMatrix * vec4(pos, 1.0f));
    currentUvs = uvs;
    currentNormal = normal;
    currentTexIndex = texIndex;
    gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(pos, 1.0);
}