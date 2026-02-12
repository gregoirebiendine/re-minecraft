#version 460 core

in vec2 currentUvs;
in vec4 atlasUvBounds;
in vec4 currentColor;
flat in uint currentLayer;

out vec4 FragColor;

uniform sampler2DArray Textures;

void main()
{
    if (currentUvs.x == -1 || currentUvs.y == -1)
        FragColor = currentColor;
    else {
        vec2 atlasUvs = mix(atlasUvBounds.xy, atlasUvBounds.zw, currentUvs);
        FragColor = textureLod(Textures, vec3(atlasUvs, currentLayer), 0.0) * currentColor;
    }
}