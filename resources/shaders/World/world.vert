#version 460 core

layout (location = 0) in ivec3 pos;
layout (location = 1) in ivec3 normal;
layout (location = 2) in ivec2 uvs;
layout (location = 3) in uint texIndex;

uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 ModelMatrix;

out vec3 currentPos;
out vec3 currentNormal;
out vec2 currentUvs;
flat out uint currentTexIndex;

void main()
{
    vec4 worldPos = vec4(vec3(pos), 1.f);
    currentPos = vec3(ModelMatrix * worldPos);
    currentUvs = vec2(uvs);
    currentNormal = vec3(normal);
    currentTexIndex = texIndex;
    gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * worldPos;
}