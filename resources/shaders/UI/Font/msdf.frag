#version 460 core

in vec2 currentUv;
in vec4 currentColor;

out vec4 FragColor;

uniform sampler2D Textures;
uniform float ScreenPxRange;

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

void main()
{
    vec3  msd = texture(Textures, currentUv).rgb;
    float sd = median(msd.r, msd.g, msd.b);
    float dist = ScreenPxRange * (sd - 0.5) + 0.5;
    float alpha = clamp(dist, 0.0, 1.0);

    FragColor = vec4(currentColor.rgb, currentColor.a * alpha);
}