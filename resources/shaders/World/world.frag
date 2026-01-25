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
    float angle = float(rotation) * 1.5708; // 90 degrees in radians
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
    // Apply UV rotation for top/bottom faces
    vec2 texCoords = currentUvs;
    if (currentNormal.y != 0.0) {
        texCoords = rotateUV(currentUvs, currentRotation);
    }

    // ambient lighting
    float ambient = 0.4f;
    vec3 lightPos = vec3(64.f, 256.f, 64.f);

    // diffuse lighting
    vec3 normal = normalize(currentNormal);
    vec3 lightDirection = normalize(lightPos - currentPos);
    float diffuse = max(dot(normal, lightDirection), 0.0f);

    FragColor = texture(Textures, vec3(texCoords, currentTexIndex)) * vec4(1.f, 1.f, 1.f, 1.f) * (diffuse + ambient);
}