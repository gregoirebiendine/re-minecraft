#version 460 core

in vec2 currentUvs;
in vec4 atlasUvBounds;
flat in uint currentLayer;

out vec4 FragColor;

uniform sampler2DArray Textures;

const float TEXEL_INSET = 2.0 / 1024.0;

void main()
{
    // Apply texel inset to atlas bounds to prevent bleeding
    vec2 atlasMin = atlasUvBounds.xy; // + TEXEL_INSET (removed because of black bleeding)
    vec2 atlasMax = atlasUvBounds.zw; // - TEXEL_INSET (removed because of black bleeding)

    // Map local UV (0-1) to inset atlas region
    vec2 atlasUvs = mix(atlasMin, atlasMax, currentUvs);

    // Texture
    FragColor = textureLod(Textures, vec3(atlasUvs, currentLayer), 0.0);
}