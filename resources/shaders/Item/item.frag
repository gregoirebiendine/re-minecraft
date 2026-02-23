#version 460 core

in vec2 currentUvs;
in vec4 atlasUvBounds;
in vec3 fragNormal;
flat in uint currentLayer;

out vec4 FragColor;

uniform sampler2DArray Textures;

void main()
{
    vec2 atlasUvs = mix(atlasUvBounds.xy, atlasUvBounds.zw, currentUvs);
    vec4 color = textureLod(Textures, vec3(atlasUvs, currentLayer), 0.0);

    if (color.a < 0.01)
        discard;

    vec3 n = normalize(gl_FrontFacing ? fragNormal : -fragNormal);
    float shade;
    if (n.y > 0.5)
        shade = 1.0;
    else if (n.y < -0.5)
        shade = 0.5;
    else if (abs(n.x) > 0.5)
        shade = 0.8;
    else
        shade = 0.9;

    FragColor = vec4(color.rgb * shade, color.a);
}