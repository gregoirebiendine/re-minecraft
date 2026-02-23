#version 460 core

in vec2 currentUvs;
in vec4 atlasUvBounds;
in float shade;
flat in uint currentLayer;

out vec4 FragColor;

uniform sampler2DArray Textures;

void main()
{
    vec2 atlasUvs = mix(atlasUvBounds.xy, atlasUvBounds.zw, currentUvs);
    vec4 color = textureLod(Textures, vec3(atlasUvs, currentLayer), 0.0);

    if (color.a < 0.01)
        discard;

    FragColor = vec4(color.rgb * shade, color.a);
}