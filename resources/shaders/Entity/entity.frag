#version 460 core

in vec2 currentUvs;
in vec4 atlasUvBounds;
flat in uint currentLayer;

out vec4 FragColor;

uniform sampler2DArray Textures;

void main()
{
    // Map local UV (0-1) to inset atlas region
    vec2 atlasUvs = mix(atlasUvBounds.xy, atlasUvBounds.zw, currentUvs);

    // Texture
    FragColor = textureLod(Textures, vec3(atlasUvs, currentLayer), 0.0);
}