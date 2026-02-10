#version 460 core

in vec3 currentPos;
in vec3 currentNormal;
in vec2 currentUvs;
in vec4 atlasUvBounds;
flat in uint currentLayer;
flat in uint currentRotation;

out vec4 FragColor;

uniform sampler2DArray Textures;

// Texel offset for 1024x1024 atlas to prevent bleeding (1 full texel inset)
const float TEXEL_INSET = 1.0 / 1024.0;

// Rotate UV coordinates by 90 degree increments
vec2 rotateUV(vec2 uv, uint rotation) {
    vec2 centered = uv - 0.5;
    float angle = -float(rotation) * 1.5708; // 90 degrees in radians
    float c = cos(angle);
    float s = sin(angle);
    vec2 rotated = vec2(
        centered.x * c - centered.y * s,
        centered.x * s + centered.y * c
    );
    return rotated + 0.5;
}

void main()
{
    // Rotate UVs 180° (flip both axes)
    vec2 localUvs = vec2(1.0) - currentUvs;

    // HORIZONTAL rotation (0-3): rotate UV on UP/DOWN faces
    if (currentNormal.y != 0.0 && currentRotation < 4u)
        localUvs = rotateUV(localUvs, currentRotation);

    // AXIS rotation (rotation 5) : rotate UV 90° on faces perpendicular to EAST/WEST
    if (currentRotation == 5u && abs(currentNormal.x) > 0.5)
        localUvs = rotateUV(localUvs, 1u);

    // X-axis (rotation 6): UP/DOWN and NORTH/SOUTH faces need rotation
    if (currentRotation == 6u && (currentNormal.y != 0.0 || abs(currentNormal.z) > 0.5))
        localUvs = rotateUV(localUvs, 1u);

    // Apply texel inset to atlas bounds to prevent bleeding
    vec2 atlasMin = atlasUvBounds.xy + TEXEL_INSET;
    vec2 atlasMax = atlasUvBounds.zw - TEXEL_INSET;

    // Map local UV (0-1) to inset atlas region
    vec2 atlasUvs = mix(atlasMin, atlasMax, localUvs);

    // ambient lighting
    float ambient = 0.4f;
    vec3 lightPos = vec3(64.f, 256.f, 64.f);

    // diffuse lighting
    vec3 normal = normalize(currentNormal);
    vec3 lightDirection = normalize(lightPos - currentPos);
    float diffuse = max(dot(normal, lightDirection), 0.0f);

    // Discard full transparency (use textureLod to avoid implicit LOD issues at distance)
    vec4 texColor = textureLod(Textures, vec3(atlasUvs, currentLayer), 0.0);
    if (texColor.a < 0.1)
        discard;
    FragColor = texColor * vec4(0.9f,0.9f,0.9f,1.f) * (diffuse + ambient);
}