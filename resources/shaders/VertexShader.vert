#version 460 core

//layout (location = 0) in vec3 basePos;
//layout (location = 1) in vec2 baseTex;
//layout (location = 2) in uint texOffset;
//layout (location = 3) in vec3 posOffset;
//layout (location = 4) in int renderedSide;

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
//layout (location = 2) in uint texOffset;

out vec2 TexCoord;

uniform mat4 ViewMatrix;
uniform mat4 TransMatrix[6];

void main()
{
    // Compute UV offset for this tile
//    vec2 computedTexOffset = vec2(0.25f * (texOffset % 4), floor(texOffset / 4) / 4);

    // Final UV coordinate
//    TexCoord = baseTex + computedTexOffset;
    TexCoord = tex;

    // Final vertex coordinate
//    vec4 worldPosition = TransMatrix[renderedSide] * vec4(basePos, 1.0);
//    worldPosition.xyz += posOffset;

    gl_Position = ViewMatrix * vec4(pos, 1.0);
}