#version 460 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in uint UVoffset;
//layout (location = 3) in vec3 POSoffset;

out vec2 TexCoord;

uniform mat4 ViewMatrix;

void main()
{
    // Compute UV offset for this tile
    vec2 off = vec2(0.25f * (UVoffset % 4), floor(UVoffset / 4) / 4);

    // Final UV coordinate
    TexCoord = tex + off;

    // Final vertex coordinate
    gl_Position = ViewMatrix * vec4(pos + vec3(gl_InstanceID, 0, 0), 1.0);
}