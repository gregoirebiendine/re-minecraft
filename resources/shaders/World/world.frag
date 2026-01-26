#version 460 core

in vec3 currentPos;
in vec3 currentNormal;
in vec2 currentUvs;
flat in uint currentTexIndex;
flat in uint currentRotation;

out vec4 FragColor;

uniform sampler2DArray Textures;

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
    vec2 texCoords = currentUvs;

    // HORIZONTAL rotation (0-3): rotate UV on UP/DOWN faces
    if (currentNormal.y != 0.0 && currentRotation < 4u)
        texCoords = rotateUV(currentUvs, currentRotation);

    // AXIS rotation (rotation 5) : rotate UV 90° on faces perpendicular to EAST/WEST
    if (currentRotation == 5u && abs(currentNormal.x) > 0.5)
        texCoords = rotateUV(currentUvs, 1u);

    // X-axis (rotation 6): UP/DOWN and NORTH/SOUTH faces need rotation
    if (currentRotation == 6u && (currentNormal.y != 0.0 || abs(currentNormal.z) > 0.5))
        texCoords = rotateUV(currentUvs, 1u);

    // ambient lighting
    float ambient = 0.4f;
    vec3 lightPos = vec3(64.f, 256.f, 64.f);

    // diffuse lighting
    vec3 normal = normalize(currentNormal);
    vec3 lightDirection = normalize(lightPos - currentPos);
    float diffuse = max(dot(normal, lightDirection), 0.0f);

    // Discard full transparency
    vec4 texColor = texture(Textures, vec3(texCoords, currentTexIndex));
    if (texColor.a < 0.1)
        discard;
    FragColor = texColor * vec4(0.9f,0.9f,0.9f,1.f) * (diffuse + ambient);
}